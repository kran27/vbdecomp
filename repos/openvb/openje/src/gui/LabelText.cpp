#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include <algorithm>
#include <cstdlib>
#include <new>
#include <d3dx8mesh.h>

namespace GUI
{
	LabelLine::LabelLine(DWORD rowId, LONG rowBegin, LONG rowEnd, LONG rowX, LONG rowY,
		LONG rowWidth, LONG rowHeight, LONG sx, LONG sy, BYTE rowHidden, BYTE rowHardBreak)
		: id(rowId), begin(rowBegin), end(rowEnd), x(rowX), y(rowY), width(rowWidth),
		height(rowHeight), horizontalScroll(sx), verticalScroll(sy), hidden(rowHidden), hardBreak(rowHardBreak)
	{}

	void Label::Measure(Font* font, char character, FontGlyph& glyph, LONG x)
	{
		font->GetGlyph(character, glyph);
		switch (character)
		{
		case '\t':
		{
			//Original IDIV is unchecked, including a zero tab-width divisor.
			const LONG remainder = x % tabWidth;
			if (remainder) glyph.width = tabWidth - remainder;
			else glyph.width = 0;
			break;
		}
		case '\n':
		case '\r':
			glyph.width = 0;
			break;
		}
	}
	BYTE Label::WordBoundary(const std::string& value, LONG index)
	{
		BYTE boundary = 0;
		if (value[index] == ' ') boundary = 1;
		if (value[index] == '\n') boundary = 1;
		if (value[index] == '\r') boundary = 1;
		if (value[index] == '\t') boundary = 1;
		if (boundary && index < static_cast<LONG>(value.size()) - 1)
		{
			if (value[index + 1] == ' ') return 0;
			if (value[index + 1] == '\n' || value[index + 1] == '\r' || value[index + 1] == '\t') return 0;
		}
		return boundary;
	}
	LONG Label::MeasureWord(const std::string& value, Font* font, LONG begin, LONG x)
	{
		LONG i = begin;
		LONG width = 0;
		const LONG length = static_cast<LONG>(value.size());
		bool inWord = false, leadingBoundary = false;
		FontGlyph glyph;
		if (i >= length) return width;
		for (;;)
		{
			if (!inWord)
			{
				if (WordBoundary(value, i)) { if (i == begin) leadingBoundary = true; }
				else inWord = true;
			}
			else if (leadingBoundary)
			{
				if (value[i] == ' ' || value[i] == '\n' || value[i] == '\r' || value[i] == '\t') break;
			}
			else if (WordBoundary(value, i)) break;
			Measure(font, value[i], glyph, x - rectangle.left);
			width += glyph.width; x += glyph.width;
			if (++i >= length) return width;
		}
		Measure(font, value[i], glyph, x - rectangle.left);
		return glyph.width + width;
	}
	void Label::ClipGlyph(const FontGlyph& glyph, LONG /*rowX*/, LONG rowY,
		LONG sx, LONG sy, LONG& x, LONG& y, LONG& width, LONG& height, float* uv)
	{
		const LONG left = rectangle.left + paddingLeft;
		const LONG top = rectangle.top + paddingTop;
		const LONG right = rectangle.right - paddingRight;
		const LONG bottom = rectangle.bottom - paddingBottom;
		width = static_cast<LONG>(glyph.width) - (sx > 0 ? sx : 0);
		if (width < 0) width = 0;
		height = glyph.height; y = rowY;
		if (uv) { uv[0] = glyph.u0; uv[1] = glyph.v0; uv[2] = glyph.u1; uv[3] = glyph.v1; }
		// Original uses FILD/FIDIV, including its zero-width NaN/Inf domain.
		if (sx > 0)
		{ if (uv) uv[0] += float(sx) / static_cast<LONG>(glyph.width) * (uv[2] - uv[0]); }
		else if (x < left)
		{
			if (uv) uv[0] += static_cast<float>(double(left - x) / static_cast<LONG>(glyph.width) * (uv[2] - uv[0]));
			width = static_cast<LONG>(glyph.width) + x - left; x = left;
			if (width < 0) width = 0;
		}
		if (x + width >= right)
		{
			if (uv) uv[2] -= static_cast<float>(double(width + x - right) / static_cast<LONG>(glyph.width) * (uv[2] - uv[0]));
			width = right - x;
		}
		if (sy > fontSize) { if (uv) uv[1] = uv[3]; height = 0; }
		else if (sy > 0)
		{
			if (uv) uv[1] += float(sy) / static_cast<LONG>(glyph.height) * (uv[3] - uv[1]);
			height = height - sy < 0 ? 0 : height - sy;
		}
		if (y < top)
		{
			if (uv) uv[1] += static_cast<float>(double(top - y) / static_cast<LONG>(glyph.height) * (uv[3] - uv[1]));
			height += y - top; y = top;
		}
		if (y + height >= bottom)
		{
			if (uv) uv[3] -= static_cast<float>((uv[3] - uv[1]) * (double(height + y - bottom) / static_cast<LONG>(glyph.height)));
			height = bottom - y;
		}
	}
	LONG Label::EmitLine(DWORD destination, LONG firstVisible, LONG begin, LONG end,
		LONG rowX, LONG rowY, LONG rowWidth, LONG sx, LONG sy, BYTE hiddenLine)
	{
		const LONG available = rectangle.right - paddingRight - paddingLeft - rectangle.left;
		LONG x, y = rowY;
		switch (alignment)
		{
		case 1: x = rowX + available - rowWidth; break;
		case 2: x = rowX + (available - rowWidth) / 2; break;
		default: x = rowX; break;
		}
		Font* font = g_Manager.GetFont(fontName, fontFlags, fontSize, fontCharset);
		LONG height = 0;
		//The original passes the first-visible text index independently from its
		//output destination; it does not skip the clipped leading characters.
		const LONG first = begin - firstVisible, last = end - firstVisible;
		for (LONG i = first; i < last; ++i)
		{
			FontGlyph glyph;
			Measure(font, text[i + firstVisible], glyph, x - rectangle.left);
			LONG width, clippedHeight; float uv[4];
			ClipGlyph(glyph, rowX, rowY, sx, sy, x, y, width, clippedHeight, uv);
			if (!hiddenLine)
			{
				const DWORD slot = destination + DWORD(i - first);
				//The original publishes each edge through the current owner;
				//there is no temporary RECT copied through a cached pointer.
				glyphRectangles[slot].left = x;
				glyphRectangles[slot].top = y;
				glyphRectangles[slot].right = x + width;
				glyphRectangles[slot].bottom = y + clippedHeight;
				//Original writes color/RHW/UV directly in this order, reloading its
				//owner between stores. XY/depth remain unwritten until later passes.
				textVertices[slot * 6].diffuse = 0xFF808080; textVertices[slot * 6].rhw = 1.0f;
				textVertices[slot * 6].u = uv[0]; textVertices[slot * 6].v = uv[1];
				textVertices[slot * 6 + 1].diffuse = 0xFF808080; textVertices[slot * 6 + 1].rhw = 1.0f;
				textVertices[slot * 6 + 1].u = uv[2]; textVertices[slot * 6 + 1].v = uv[1];
				textVertices[slot * 6 + 2].diffuse = 0xFF808080; textVertices[slot * 6 + 2].rhw = 1.0f;
				textVertices[slot * 6 + 2].u = uv[0]; textVertices[slot * 6 + 2].v = uv[3];
				textVertices[slot * 6 + 3].diffuse = 0xFF808080; textVertices[slot * 6 + 3].rhw = 1.0f;
				textVertices[slot * 6 + 3].u = uv[0]; textVertices[slot * 6 + 3].v = uv[3];
				textVertices[slot * 6 + 4].diffuse = 0xFF808080; textVertices[slot * 6 + 4].rhw = 1.0f;
				textVertices[slot * 6 + 4].u = uv[2]; textVertices[slot * 6 + 4].v = uv[1];
				textVertices[slot * 6 + 5].diffuse = 0xFF808080; textVertices[slot * 6 + 5].rhw = 1.0f;
				textVertices[slot * 6 + 5].u = uv[2]; textVertices[slot * 6 + 5].v = uv[3];
			}
			x += width; sx -= glyph.width;
			if (clippedHeight > height) height = clippedHeight;
		}
		return height;
	}

	void Label::RebuildText()
	{
		ClearGeometry(); textHeight = 0;
		//Real VC7 vector::clear calls _Tidy, releasing the allocation and
		//resetting all three pointers before missing-font/empty-text exits.
		lines.clear();
		visibleLines = 0; firstVisibleCharacter = -1;
		const LONG left = rectangle.left + paddingLeft, top = rectangle.top + paddingTop;
		const LONG right = rectangle.right - paddingRight, bottom = rectangle.bottom - paddingBottom;
		//The original snapshots both the row origin and its advancing position
		//before the font lookup; later row breaks restore this captured origin.
		LONG x = left, y = top;
		Font* font = g_Manager.GetFont(fontName, fontFlags, fontSize, fontCharset);
		if (!font) return;
		const LONG length = static_cast<LONG>(text.size());
		LONG initialHorizontal, sy;
		if (length)
		{ initialHorizontal = horizontalScroll; sy = verticalScroll; displayedCharacters = length; }
		if (length <= 0)
		{
			FontGlyph glyph; Measure(font, ' ', glyph, left - rectangle.left);
			LONG offset = 0;
			switch (verticalAlignment)
			{
			case 1: offset = bottom - static_cast<LONG>(glyph.height) - top; break;
			//The original divides the signed available height, including negatives.
			case 2: offset = (bottom - static_cast<LONG>(glyph.height) - top) / 2; break;
			}
			LabelLine line;
			line.height = glyph.height;
			line.id = 0; line.begin = 0; line.end = 0; line.width = 0;
			line.hardBreak = 0; line.y = top + offset;
			line.verticalScroll = verticalScroll; line.x = left;
			line.horizontalScroll = horizontalScroll; line.hidden = 1;
			lines.push_back(line);
			visibleLines = 1; firstVisibleCharacter = 0; textHeight = glyph.height;
			return;
		}
		//All buffers are allocated from complete text length, before layout.
		//The original uses scalar operator new and later real free, with no
		//catch, allocation fallback, zero fill or emitted-count shrink.
		glyphRectangles = static_cast<RECT*>(::operator new(sizeof(RECT) * length));
		textVertexCount = 6 * length;
		textVertices = static_cast<JE::Gfx::UIVertex*>(::operator new(sizeof(JE::Gfx::UIVertex) * textVertexCount));
		DWORD* textures = static_cast<DWORD*>(::operator new(sizeof(DWORD) * length));
		LONG rowHeight = 0, rowWidth = 0, begin = 0;
		bool word = false; BYTE hiddenLine = 0;
		totalHeight = 0; maximumRowWidth = 0; textHeight = 0; textWidth = 0;
		lines.clear(); //Original calls the real row clear again after raw buffer allocation.
		const LONG advanceScroll = initialHorizontal <= 0 ? 0 : initialHorizontal;
		FontGlyph glyph;
		LONG i;
		for (i = 0; i < length; ++i)
		{
			Measure(font, text[i], glyph, x - rectangle.left);
			const LONG advance = glyph.width - advanceScroll;
			textures[i] = glyph.texture;
			if (static_cast<LONG>(glyph.height) > rowHeight) rowHeight = glyph.height;
			const bool newline = text[i] == '\n' || text[i] == '\r';
			if (wrap)
			{
				if (word || newline)
				{
					if ((WordBoundary(text, i) || newline) && (x + MeasureWord(text, font, i, x) >= right || newline))
					{
						if (rowWidth > maximumRowWidth) maximumRowWidth = rowWidth;
						if (!hiddenLine && x - left > textWidth) textWidth = x - left;
						lines.push_back(LabelLine(static_cast<DWORD>(lines.size()), begin, i, left, y,
							rowWidth, rowHeight, horizontalScroll, sy, hiddenLine, 1));
						rowWidth = 0; begin = i + 1; x = left;
						y += sy <= rowHeight ? rowHeight - sy : 0; totalHeight += rowHeight;
						sy = sy - rowHeight < 0 ? 0 : sy - rowHeight;
						if (!verticalAlignment && y >= bottom && !hiddenLine) hiddenLine = 1;
						word = false; continue;
					}
				}
				else if (!WordBoundary(text, i)) word = true;
				if (x + advance >= right)
				{
					if (rowWidth > maximumRowWidth) maximumRowWidth = rowWidth;
					if (!hiddenLine && x - left > textWidth) textWidth = x - left;
					const std::string& source = text;
					if (text[i] == ' ' || source[i] == '\t' || source[i] == '\n' || source[i] == '\r') ++i;
					lines.push_back(LabelLine(static_cast<DWORD>(lines.size()), begin, i, left, y,
						rowWidth, rowHeight, horizontalScroll, sy, hiddenLine, 0));
					rowWidth = 0; begin = i; x = left;
					y += sy <= rowHeight ? rowHeight - sy : 0; totalHeight += rowHeight;
					sy = sy - rowHeight < 0 ? 0 : sy - rowHeight;
					if (!verticalAlignment && y >= bottom)
					{
						if (!hiddenLine) hiddenLine = 1;
						continue;
					}
					word = false;
				}
			}
			else if (x >= right)
			{
				if (!hiddenLine)
				{
					lines.push_back(LabelLine(static_cast<DWORD>(lines.size()), begin, i, left, y,
						rowWidth, rowHeight, horizontalScroll, sy, 0, 0));
					hiddenLine = 1;
				}
				continue;
			}
			if (!word && y != top && newline) continue;
			rowWidth += glyph.width; x += advance;
		}
		totalHeight += rowHeight; textHeight += rowHeight;
		if (rowWidth > maximumRowWidth) maximumRowWidth = rowWidth;
		if (x - left > textWidth) textWidth = x - left;
		LabelLine lastLine;
		lastLine.id = static_cast<DWORD>(lines.size()); lastLine.begin = begin;
		lastLine.width = rowWidth; lastLine.horizontalScroll = horizontalScroll;
		lastLine.y = y; lastLine.hidden = hiddenLine; lastLine.end = i;
		lastLine.x = left; lastLine.height = rowHeight; lastLine.verticalScroll = sy;
		lastLine.hardBreak = 0;
		lines.push_back(lastLine);
		LONG yOffset = 0;
		switch (verticalAlignment)
		{
		case 1: yOffset = bottom - totalHeight - top; break;
		case 2: yOffset = (bottom - totalHeight - top) / 2; break;
		}
		LONG emittedGlyphs = 0;
		for (std::vector<LabelLine>::iterator row = lines.begin(); row != lines.end(); ++row)
		{
			LabelLine& line = *row; line.y += yOffset;
			if (firstVisibleCharacter < 0 && line.height + line.y > top && line.verticalScroll < line.height)
			{
				firstVisibleCharacter = line.begin;
				LONG first = line.begin, scroll = line.horizontalScroll;
				const LONG available = right - left;
				LONG firstX = line.x;
				switch (alignment)
				{
				case 1: firstX = available + (line.x - line.width); break;
				case 2: firstX = line.x + (available - line.width) / 2; break;
				}
				for (; first < line.end; ++first)
				{
					if (scroll <= 0)
					{
						firstVisibleCharacter = first;
						break;
					}
					FontGlyph glyph; Measure(font, text[first], glyph, firstX - rectangle.left);
					scroll -= glyph.width;
				}
			}
			if (firstVisibleCharacter >= 0)
			{
				const LONG height = EmitLine(emittedGlyphs, firstVisibleCharacter, line.begin, line.end,
					line.x, line.y, line.width, line.horizontalScroll, line.verticalScroll, line.hidden);
				if (!line.hidden) { emittedGlyphs += line.end - line.begin; ++visibleLines; }
				textHeight += height;
			}
		}
		// The original insertion loop starts at2, leaving the first pair alone.
		for (LONG at = 2; at < emittedGlyphs; ++at)
		{
			const DWORD texture = textures[at]; const RECT rect = glyphRectangles[at];
			const JE::Gfx::UIVertex vertex0 = textVertices[at * 6];
			const JE::Gfx::UIVertex vertex1 = textVertices[at * 6 + 1];
			const JE::Gfx::UIVertex vertex2 = textVertices[at * 6 + 2];
			const JE::Gfx::UIVertex vertex3 = textVertices[at * 6 + 3];
			const JE::Gfx::UIVertex vertex4 = textVertices[at * 6 + 4];
			const JE::Gfx::UIVertex vertex5 = textVertices[at * 6 + 5];
			LONG dest = at;
			while (texture < textures[dest - 1])
			{
				textures[dest] = textures[dest - 1]; glyphRectangles[dest] = glyphRectangles[dest - 1];
				textVertices[dest * 6] = textVertices[(dest - 1) * 6];
				textVertices[dest * 6 + 1] = textVertices[(dest - 1) * 6 + 1];
				textVertices[dest * 6 + 2] = textVertices[(dest - 1) * 6 + 2];
				textVertices[dest * 6 + 3] = textVertices[(dest - 1) * 6 + 3];
				textVertices[dest * 6 + 4] = textVertices[(dest - 1) * 6 + 4];
				textVertices[dest * 6 + 5] = textVertices[(dest - 1) * 6 + 5];
				--dest; //Original has no lower-bound check before the next predecessor read.
			}
			textures[dest] = texture; glyphRectangles[dest] = rect;
			textVertices[dest * 6] = vertex0;
			textVertices[dest * 6 + 1] = vertex1;
			textVertices[dest * 6 + 2] = vertex2;
			textVertices[dest * 6 + 3] = vertex3;
			textVertices[dest * 6 + 4] = vertex4;
			textVertices[dest * 6 + 5] = vertex5;
		}
		LONG groups = 1;
		for (LONG j = 1; j < emittedGlyphs; ++j) if (textures[j] != textures[j - 1]) ++groups;
		textMesh.vertices = textVertices; textPrimitives = textMesh.SetPrimitiveCount(groups);
		LONG beginGlyph = 0, endGlyph = 0;
		LONG group;
		for (group = 0; group < groups; ++group)
		{
			textPrimitives[group].type = D3DPT_TRIANGLELIST;
			textPrimitives[group].value28 = beginGlyph;
			Gfx_Material& material = textMesh.materialSet.materials[group];
			material.textureHandles[0] = textures[endGlyph];
			material.style1 = 20; material.style2 = 20;
			++endGlyph;
			material.mode28 = 1; material.mode24 = 1;
			//The original advances one run at a time. Only a differing next
			//texture publishes this group's count; the terminal run falls
			//through to the final publication after the outer traversal.
			for (; endGlyph < emittedGlyphs; ++endGlyph)
			{
				if (textures[endGlyph] != textures[endGlyph - 1])
				{
					textPrimitives[group].value30 = 2 * (endGlyph - beginGlyph);
					beginGlyph = endGlyph;
					break;
				}
			}
		}
		// Original publishes the last count after all material setup, even when
		// no glyph was emitted (one group, one nominal triangle pair).
		//Use the completed traversal index, including the original zero index
		//when a nonpositive group count bypasses the loop.
		textPrimitives[group - 1].value30 = 2 * (endGlyph - beginGlyph);
		textMesh.SetVertexDesc(0x144, D3DXGetFVFVertexSize(0x144), 6 * emittedGlyphs);
		if (textColor != 0xFF808080) textMesh.materialSet.SetColors(1, 1, textColor, 0);
		textMesh.materialSet.RebuildGroups();
		std::free(textures);
		geometryDirty = 1; textGeometryDirty = 1; g_Manager.MarkDepthDirty();
	}
}
