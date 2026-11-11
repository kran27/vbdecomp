#include "gui/Edit.hpp"
#include "gui/Manager.hpp"
#include "gui/PacketManager.hpp"
#include <algorithm>
#include <cstdlib>
#include <new>
#include <mmsystem.h>

namespace GUI
{
	LONG Edit::CaretLine() const
	{
		LONG line = FindLine(caretPosition);
		if (line == firstSelectionLine + 1) --line;
		return line;
	}
	LONG Edit::LineOrigin(const LabelLine& line) const
	{
		LONG available = rectangle.right - paddingRight - rectangle.left - paddingLeft;
		if (alignment == 1) return line.x + available - line.width;
		if (alignment == 2) return line.x + (available - line.width) / 2;
		return line.x;
	}
	void Edit::ClipLine(const LabelLine& line, LONG& x, LONG& y, LONG& width, LONG& height) const
	{
		const LONG left = LONG(DWORD(rectangle.left) + DWORD(paddingLeft));
		const LONG top = LONG(DWORD(rectangle.top) + DWORD(paddingTop));
		const LONG right = LONG(DWORD(rectangle.right) - DWORD(paddingRight));
		const LONG bottom = LONG(DWORD(rectangle.bottom) - DWORD(paddingBottom));
		width = LONG(DWORD(line.width) - DWORD(line.horizontalScroll <= 0 ? 0 : line.horizontalScroll));
		if (width < 0) width = 0;
		height = line.height;
		x = line.x;
		y = line.y;
		if (line.horizontalScroll <= 0 && x < left)
		{
			width = LONG(DWORD(line.width) + DWORD(x) - DWORD(left));
			x = left;
			if (width < 0) width = 0;
		}
		if (LONG(DWORD(x) + DWORD(width)) >= right) width = LONG(DWORD(right) - DWORD(x));
		const LONG scroll = line.verticalScroll;
		if (scroll > line.height) height = 0;
		else if (scroll > 0) height = LONG(DWORD(height) - DWORD(scroll)) < 0 ? 0 : LONG(DWORD(height) - DWORD(scroll));
		if (y < top) { height = LONG(DWORD(height) + DWORD(y) - DWORD(top)); y = top; }
		if (LONG(DWORD(y) + DWORD(height)) >= bottom) height = LONG(DWORD(bottom) - DWORD(y));
	}
	void Edit::BuildCaretGeometry() //4C2390, complete original builder; no shared replacement builder.
	{
		caretGeometry.active = 0;
		LONG first = FindLine(firstVisibleCharacter);
		if (first == firstSelectionLine + 1) --first;
		std::list<LabelLine*> selectedLines;
		const LONG begin = caretPosition, end = caretPosition;
		for (LONG i = 0; i < LONG(visibleLines); ++i)
		{
			const LONG index = first + i;
			LabelLine* line = (index >= 0 && index < LONG(lines.size())) ? &lines[index] : 0;
			if (!line) continue;
			LONG last = line->end;
			if (firstSelectionLine >= 0 && line->hardBreak) ++last;
			if ((line->begin >= begin && line->begin <= end) || (last >= begin && last <= end)
				|| (begin >= line->begin && begin <= last) || (end >= line->begin && end <= last))
				selectedLines.push_back(line);
		}
		LONG writtenVertices = 0, writtenIndices = 0;
		EditGeometry& geometry = caretGeometry;
		if (LONG(selectedLines.size()) <= 0)
		{
			//Original no-row branch changes only the vertex count. The retained
			//arrays, capacities, index count and mesh aliases are left alone.
			geometry.vertexCount = 0;
			return;
		}
		LONG required = 4 * LONG(selectedLines.size());
		if (required > geometry.vertexCapacity)
		{
			if (geometry.vertices) { std::free(geometry.vertices); geometry.vertices = 0; }
			LONG capacity = geometry.vertexCapacity + 20;
			if (required > capacity) capacity = required;
			geometry.vertexCapacity = capacity;
			if (capacity > 0) geometry.vertices = static_cast<EditVertex*>(::operator new(sizeof(EditVertex) * capacity));
		}
		geometry.vertexCount = required;
		required = 6 * LONG(selectedLines.size());
		if (required > geometry.indexCapacity)
		{
			if (geometry.indices) { std::free(geometry.indices); geometry.indices = 0; }
			LONG capacity = geometry.indexCapacity + 30;
			if (required > capacity) capacity = required;
			geometry.indexCapacity = capacity;
			if (capacity > 0) geometry.indices = static_cast<WORD*>(::operator new(sizeof(WORD) * capacity));
		}
		geometry.indexCount = required;
		float sx, sy;
		g_Manager.GetScale(sx, sy);
		const LONG left = rectangle.left + paddingLeft, right = rectangle.right - paddingRight;
		const LONG available = right - left;
		Font* font = g_Manager.GetFont(fontName, fontFlags, fontSize, fontCharset);
		for (std::list<LabelLine*>::iterator i = selectedLines.begin(); i != selectedLines.end(); ++i)
		{
			LabelLine& line = **i;
			LONG clippedX, clippedY, clippedWidth, clippedHeight;
			ClipLine(line, clippedX, clippedY, clippedWidth, clippedHeight);
			std::string row;
			GetLineText(line.id, row);
			LONG startX = clippedX, endX = clippedX;
			const LONG top = clippedY, bottom = clippedY + clippedHeight;
			LONG origin = line.x;
			if (alignment == 1) origin = line.x + available - line.width;
			else if (alignment == 2) origin = line.x + (available - line.width) / 2;
			const LONG rowBegin = line.begin;
			LONG rowEnd = line.end;
			if (line.hardBreak) { row.append(1, ' '); ++rowEnd; }
			LONG startPosition = begin;
			if (startPosition <= rowBegin) startPosition = rowBegin;
			LONG endPosition = end;
			if (endPosition >= rowEnd) endPosition = rowEnd;
			LONG scrollX, scrollY;
			GetScroll(scrollX, scrollY); //Actual virtual+19C, rather than bypassing it with field reads.
			if (startPosition >= rowBegin + (LONG(line.id) == firstSelectionLine + 1))
			{
				BOOL excluded = FALSE;
				if (LONG(line.id) != firstSelectionLine)
					if (line.id != lines.size() - 1 && !line.hardBreak) excluded = TRUE;
				if (startPosition <= rowEnd - excluded)
				{
					LONG x = line.x;
					const LONG relative = origin - left;
					FontGlyph startGlyph;
					Measure(font, row.c_str()[startPosition - rowBegin], startGlyph, relative);
					if (startPosition > rowBegin)
						x += MeasureAdvance(row.substr(0, startPosition - rowBegin), font, relative);
					LONG y, width, height;
					ClipGlyph(startGlyph, line.x, line.y, line.horizontalScroll, line.verticalScroll,
						x, y, width, height, 0);
					startX = x - scrollX;
				}
			}
			if (endPosition >= rowBegin + (LONG(line.id) == firstSelectionLine + 1))
			{
				BOOL excluded = FALSE;
				if (LONG(line.id) != firstSelectionLine)
					if (line.id != lines.size() - 1 && !line.hardBreak) excluded = TRUE;
				if (endPosition <= rowEnd - excluded)
				{
					LONG x = line.x;
					const LONG relative = origin - left;
					FontGlyph endGlyph;
					Measure(font, row.c_str()[endPosition - rowBegin], endGlyph, relative);
					if (endPosition > rowBegin)
						x += MeasureAdvance(row.substr(0, endPosition - rowBegin), font, relative);
					LONG y, width, height;
					ClipGlyph(endGlyph, line.x, line.y, line.horizontalScroll, line.verticalScroll,
						x, y, width, height, 0);
				LONG caretWidth = endGlyph.width;
				if (selectionAnchor != caretPosition || insertMode) caretWidth = 2;
					endX = x + caretWidth - scrollX;
				}
			}
			if (startX < left) startX = left;
			if (startX > right) startX = right;
			if (endX < left) endX = left;
			if (endX > right) endX = right;
			if (startX == endX) continue;
			const float x0 = float(startX * sx), y0 = float(top * sy);
			const float x1 = float(endX * sx), y1 = float(bottom * sy);
			if (DWORD(writtenVertices) <= DWORD(geometry.vertexCapacity - 4))
			{
				EditVertex* vertex = geometry.vertices + writtenVertices;
				vertex[0].x = x0; vertex[0].y = y0;
				vertex[1].x = x1; vertex[1].y = y0;
				vertex[2].x = x0; vertex[2].y = y1;
				vertex[3].x = x1; vertex[3].y = y1;
				vertex[0].diffuse = caretColor;
				vertex[1].diffuse = caretColor;
				vertex[2].diffuse = caretColor;
				vertex[3].diffuse = caretColor;
				//Z stays unwritten until the depth pass, as in both originals.
				vertex[0].rhw = vertex[1].rhw = vertex[2].rhw = vertex[3].rhw = 1.0f;
				if (writtenIndices <= geometry.indexCapacity - 6)
				{
					geometry.indices[writtenIndices] = WORD(writtenVertices + 3);
					geometry.indices[writtenIndices + 1] = WORD(writtenVertices + 2);
					geometry.indices[writtenIndices + 2] = WORD(writtenVertices);
					geometry.indices[writtenIndices + 3] = WORD(writtenVertices + 3);
					geometry.indices[writtenIndices + 4] = WORD(writtenVertices);
					geometry.indices[writtenIndices + 5] = WORD(writtenVertices + 1);
					writtenIndices += 6;
				}
				writtenVertices += 4;
			}
		}
		geometry.mesh.vertices = geometry.vertices;
		geometry.vertexCount = writtenVertices;
		geometry.indexCount = writtenIndices;
		geometry.mesh.value00 = 0;
		geometry.mesh.value04 = DWORD(-1);
		geometry.mesh.value10 = reinterpret_cast<DWORD>(geometry.indices);
		geometry.mesh.SetVertexDesc(0x44, D3DXGetFVFVertexSize(0x44), writtenVertices);
		geometry.mesh.SetIndexDesc(2, geometry.indexCount);
		geometry.mesh.materialSet.count = 0;
		geometry.mesh.materialSet.ResetGroups();
		if (geometry.mesh.materialSet.materials)
		{
			delete[] geometry.mesh.materialSet.materials;
			geometry.mesh.materialSet.materials = 0;
		}
		Gfx_Primitive* primitive = geometry.mesh.SetPrimitiveCount(1);
		primitive->value00 = 0;
		primitive->type = D3DPT_TRIANGLELIST;
		primitive->value24 = 1;
		Gfx_Material material;
		material.style1 = material.style2 = 15;
		material.mode28 = 1;
		material.flagBC = 0;
		geometry.mesh.materialSet.materials[0] = material;
		primitive->value28 = primitive->value2C = 0;
		primitive->value30 = writtenVertices / 2;
		geometry.packet->transform = &geometry.transform;
		geometry.packet->transformEnabled = 1;
		geometry.mesh.materialSet.RebuildGroups();
		geometry.mesh.value04 = DWORD(-1);
		geometry.instances[0].SetMesh(&geometry.mesh);
		geometry.instances[0].value0C = reinterpret_cast<DWORD>(geometry.packet);
		geometry.active = 1;
		editDepthDirty = 1;
	}
	void Edit::BuildSelectionGeometry() //4C2F20, complete original builder; no shared replacement builder.
	{
		selectionGeometry.active = 0;
		if (selectionAnchor == caretPosition) return;
		LONG first = FindLine(firstVisibleCharacter);
		if (first == firstSelectionLine + 1) --first;
		std::list<LabelLine*> selectedLines;
		LONG begin = selectionAnchor;
		if (begin >= caretPosition) begin = caretPosition;
		LONG end = selectionAnchor;
		if (end <= caretPosition) end = caretPosition;
		for (LONG i = 0; i < LONG(visibleLines); ++i)
		{
			const LONG index = first + i;
			//The original selection path dereferences the returned line directly.
			LabelLine* line = (index >= 0 && index < LONG(lines.size())) ? &lines[index] : 0;
			LONG last = line->end;
			if (firstSelectionLine >= 0 && line->hardBreak) ++last;
			if ((line->begin >= begin && line->begin <= end) || (last >= begin && last <= end)
				|| (begin >= line->begin && begin <= last) || (end >= line->begin && end <= last))
				selectedLines.push_back(line);
		}
		LONG writtenVertices = 0, writtenIndices = 0;
		EditGeometry& geometry = selectionGeometry;
		if (LONG(selectedLines.size()) <= 0)
		{
			//Original no-row branch changes only the vertex count. The retained
			//arrays, capacities, index count and mesh aliases are left alone.
			geometry.vertexCount = 0;
			return;
		}
		LONG required = 4 * LONG(selectedLines.size());
		if (required > geometry.vertexCapacity)
		{
			if (geometry.vertices) { std::free(geometry.vertices); geometry.vertices = 0; }
			LONG capacity = geometry.vertexCapacity + 20;
			if (required > capacity) capacity = required;
			geometry.vertexCapacity = capacity;
			if (capacity > 0) geometry.vertices = static_cast<EditVertex*>(::operator new(sizeof(EditVertex) * capacity));
		}
		geometry.vertexCount = required;
		required = 6 * LONG(selectedLines.size());
		if (required > geometry.indexCapacity)
		{
			if (geometry.indices) { std::free(geometry.indices); geometry.indices = 0; }
			LONG capacity = geometry.indexCapacity + 30;
			if (required > capacity) capacity = required;
			geometry.indexCapacity = capacity;
			if (capacity > 0) geometry.indices = static_cast<WORD*>(::operator new(sizeof(WORD) * capacity));
		}
		geometry.indexCount = required;
		float sx, sy;
		g_Manager.GetScale(sx, sy);
		const LONG left = rectangle.left + paddingLeft, right = rectangle.right - paddingRight;
		const LONG available = right - left;
		Font* font = g_Manager.GetFont(fontName, fontFlags, fontSize, fontCharset);
		for (std::list<LabelLine*>::iterator i = selectedLines.begin(); i != selectedLines.end(); ++i)
		{
			LabelLine& line = **i;
			LONG clippedX, clippedY, clippedWidth, clippedHeight;
			ClipLine(line, clippedX, clippedY, clippedWidth, clippedHeight);
			std::string row;
			GetLineText(line.id, row);
			LONG startX = clippedX, endX = clippedX;
			const LONG top = clippedY, bottom = clippedY + clippedHeight;
			LONG origin = line.x;
			if (alignment == 1) origin = line.x + available - line.width;
			else if (alignment == 2) origin = line.x + (available - line.width) / 2;
			const LONG rowBegin = line.begin;
			LONG rowEnd = line.end;
			if (line.hardBreak) { row.append(1, ' '); ++rowEnd; }
			LONG startPosition = begin;
			if (startPosition <= rowBegin) startPosition = rowBegin;
			LONG endPosition = end;
			if (endPosition >= rowEnd) endPosition = rowEnd;
			LONG scrollX, scrollY;
			GetScroll(scrollX, scrollY); //Actual virtual+19C, rather than bypassing it with field reads.
			if (startPosition >= rowBegin + (LONG(line.id) == firstSelectionLine + 1))
			{
				BOOL excluded = FALSE;
				if (LONG(line.id) != firstSelectionLine && begin == end)
					if (line.id != lines.size() - 1 && !line.hardBreak) excluded = TRUE;
				if (startPosition <= rowEnd - excluded)
				{
					LONG x = line.x;
					const LONG relative = origin - left;
					FontGlyph startGlyph;
					Measure(font, row.c_str()[startPosition - rowBegin], startGlyph, relative);
					if (startPosition > rowBegin)
						x += MeasureAdvance(row.substr(0, startPosition - rowBegin), font, relative);
					LONG y, width, height;
					ClipGlyph(startGlyph, line.x, line.y, line.horizontalScroll, line.verticalScroll,
						x, y, width, height, 0);
					startX = x - scrollX;
				}
			}
			if (endPosition >= rowBegin + (LONG(line.id) == firstSelectionLine + 1))
			{
				BOOL excluded = FALSE;
				if (LONG(line.id) != firstSelectionLine && begin == end)
					if (line.id != lines.size() - 1 && !line.hardBreak) excluded = TRUE;
				if (endPosition <= rowEnd - excluded)
				{
					LONG x = line.x;
					const LONG relative = origin - left;
					FontGlyph endGlyph;
					Measure(font, row.c_str()[endPosition - rowBegin], endGlyph, relative);
					if (endPosition > rowBegin)
						x += MeasureAdvance(row.substr(0, endPosition - rowBegin), font, relative);
					LONG y, width, height;
					ClipGlyph(endGlyph, line.x, line.y, line.horizontalScroll, line.verticalScroll,
						x, y, width, height, 0);
					endX = x - scrollX;
				}
			}
			if (startX < left) startX = left;
			if (startX > right) startX = right;
			if (endX < left) endX = left;
			if (endX > right) endX = right;
			if (startX == endX) continue;
			const float x0 = float(startX * sx), y0 = float(top * sy);
			const float x1 = float(endX * sx), y1 = float(bottom * sy);
			if (DWORD(writtenVertices) <= DWORD(geometry.vertexCapacity - 4))
			{
				EditVertex* vertex = geometry.vertices + writtenVertices;
				vertex[0].x = x0; vertex[0].y = y0;
				vertex[1].x = x1; vertex[1].y = y0;
				vertex[2].x = x0; vertex[2].y = y1;
				vertex[3].x = x1; vertex[3].y = y1;
				vertex[0].diffuse = selectionColor;
				vertex[1].diffuse = selectionColor;
				vertex[2].diffuse = selectionColor;
				vertex[3].diffuse = selectionColor;
				//Z stays unwritten until the depth pass, as in both originals.
				vertex[0].rhw = vertex[1].rhw = vertex[2].rhw = vertex[3].rhw = 1.0f;
				if (writtenIndices <= geometry.indexCapacity - 6)
				{
					geometry.indices[writtenIndices] = WORD(writtenVertices + 3);
					geometry.indices[writtenIndices + 1] = WORD(writtenVertices + 2);
					geometry.indices[writtenIndices + 2] = WORD(writtenVertices);
					geometry.indices[writtenIndices + 3] = WORD(writtenVertices + 3);
					geometry.indices[writtenIndices + 4] = WORD(writtenVertices);
					geometry.indices[writtenIndices + 5] = WORD(writtenVertices + 1);
					writtenIndices += 6;
				}
				writtenVertices += 4;
			}
		}
		geometry.mesh.vertices = geometry.vertices;
		geometry.vertexCount = writtenVertices;
		geometry.indexCount = writtenIndices;
		geometry.mesh.value00 = 0;
		geometry.mesh.value04 = DWORD(-1);
		geometry.mesh.value10 = reinterpret_cast<DWORD>(geometry.indices);
		geometry.mesh.SetVertexDesc(0x44, D3DXGetFVFVertexSize(0x44), writtenVertices);
		geometry.mesh.SetIndexDesc(2, geometry.indexCount);
		geometry.mesh.materialSet.count = 0;
		geometry.mesh.materialSet.ResetGroups();
		if (geometry.mesh.materialSet.materials)
		{
			delete[] geometry.mesh.materialSet.materials;
			geometry.mesh.materialSet.materials = 0;
		}
		Gfx_Primitive* primitive = geometry.mesh.SetPrimitiveCount(1);
		primitive->value00 = 0;
		primitive->type = D3DPT_TRIANGLELIST;
		primitive->value24 = 1;
		Gfx_Material material;
		material.style1 = material.style2 = 15;
		material.mode28 = 1;
		material.flagBC = 0;
		geometry.mesh.materialSet.materials[0] = material;
		primitive->value28 = primitive->value2C = 0;
		primitive->value30 = writtenVertices / 2;
		geometry.packet->transform = &geometry.transform;
		geometry.packet->transformEnabled = 1;
		geometry.mesh.materialSet.RebuildGroups();
		geometry.mesh.value04 = DWORD(-1);
		geometry.instances[0].SetMesh(&geometry.mesh);
		geometry.instances[0].value0C = reinterpret_cast<DWORD>(geometry.packet);
		geometry.active = 1;
		editDepthDirty = 1;
	}
	void Edit::Draw() //4C0720.
	{
		if (editDepthDirty)
		{
			UpdateEditDepth(editDepth, editDepthIncrement);
			editDepthDirty = 0;
		}
		Label::Draw();
		if (selectionGeometry.active && !readOnly) g_Manager.packets->Submit(selectionGeometry.instances, NULL, 2);
		if (caretGeometry.active && !readOnly && (timeGetTime() - blinkTime) % 1000 < 500)
			g_Manager.packets->Submit(caretGeometry.instances, NULL, 2);
	}
	float Edit::UpdateEditDepth(float depth, float increment) //4C0510.
	{
		for (LONG i = 0; i < caretGeometry.vertexCount; ++i) caretGeometry.vertices[i].z = depth;
		const float selectionDepth = depth + increment * 0.5f;
		for (LONG i = 0; i < selectionGeometry.vertexCount; ++i)
			selectionGeometry.vertices[i].z = selectionDepth;
		return depth + increment;
	}

}
