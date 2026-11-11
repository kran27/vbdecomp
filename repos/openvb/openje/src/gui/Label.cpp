#include "gui/Label.hpp"
#include "gui/FileString.hpp"
#include "gui/Manager.hpp"
#include "gui/PacketManager.hpp"
#include "graphics/SystemInterface.hpp"
#include <cstdlib>
#include <cstdio>

namespace GUI
{
	Label::Label(DWORD kind) : Window(kind), stringReference(0), fontName("system"),
		fontFlags(0), fontSize(10), fontCharset(1), textColor(0xFF808080), textFrame(0),
		wrap(0), alignment(0), displayedCharacters(0), glyphRectangles(0),
		textPrimitives(0), textVertexCount(0), textVertices(0), textGeometryDirty(1),
		paddingLeft(0), paddingRight(0), paddingTop(0), paddingBottom(0), totalHeight(0),
		maximumRowWidth(0), textHeight(0), textWidth(0), horizontalScroll(0), verticalScroll(0),
		verticalAlignment(0), tabWidth(40), firstVisibleCharacter(-1), visibleLines(0)
	{
		SetValue3B(1);
		SetFrameVisible(0);
		textMesh.value00 = 0;
		textMesh.value04 = 0xFFFFFFFF;
		textInstance.SetMesh(&textMesh);
	}

	Label::~Label() { ClearGeometry(); }
	BYTE Label::WriteFile(FILE* file) //4A7990; always writes the current verticalAlignment field.
	{
		return Window::WriteFile(file)
			&& std::fwrite(&stringReference, 4, 1, file) == 1
			&& WriteFileString(fontName, file)
			&& std::fwrite(&fontFlags, 4, 1, file) == 1
			&& std::fwrite(&fontSize, 4, 1, file) == 1
			&& std::fwrite(&fontCharset, 4, 1, file) == 1
			&& std::fwrite(&textColor, 4, 1, file) == 1
			&& std::fwrite(&textFrame, 1, 1, file) == 1
			&& std::fwrite(&wrap, 1, 1, file) == 1
			&& std::fwrite(&alignment, 4, 1, file) == 1
			&& std::fwrite(&paddingLeft, 4, 1, file) == 1
			&& std::fwrite(&paddingRight, 4, 1, file) == 1
			&& std::fwrite(&paddingTop, 4, 1, file) == 1
			&& std::fwrite(&paddingBottom, 4, 1, file) == 1
			&& std::fwrite(&verticalAlignment, 4, 1, file) == 1;
	}

	void Label::ClearGeometry()
	{
		textMesh.Free();
		displayedCharacters = 0;
		std::free(glyphRectangles);
		glyphRectangles = 0;
		textVertexCount = 0;
		std::free(textVertices);
		textVertices = 0;
	}
	void Label::ApplySkin(const std::string& value) { Window::ApplyProfile(value, "label"); }
	void Label::SetText(const std::string& value) { stringReference = 0; text = value; RebuildText(); }
	void Label::SetFont(const std::string& face, DWORD flags, LONG size, DWORD charset)
	{
		fontName = face; fontFlags = flags; fontSize = size; fontCharset = charset;
		RebuildText();
	}
	void Label::SetWrap(BYTE value) { wrap = value; RebuildText(); }
	void Label::SetAlignment(DWORD value) { alignment = value; RebuildText(); }
	void Label::SetVerticalAlignment(DWORD value) { verticalAlignment = value; RebuildText(); }
	void Label::Resize(LONG width, LONG height) { Window::Resize(width, height); RebuildText(); }
	LONG Label::GetTextWidth() const { return textWidth; }
	LONG Label::GetTextHeight() const { return totalHeight; }
	void Label::SetTextFrame(BYTE value)
	{
		textFrame = value;
		Window::SetFrameVisible(value);
		SetCenterVisible(textFrame);
	}
	void Label::SetTextColor(DWORD value)
	{
		textMesh.materialSet.SetColors(1, 1, value, 0);
		if ((textColor & 0xFF000000) == 0xFF000000)
		{
			if ((value & 0xFF000000) != (textColor & 0xFF000000))
				textMesh.materialSet.RebuildGroups();
		}
		else if ((value & 0xFF000000) == 0xFF000000)
			textMesh.materialSet.RebuildGroups();
		textColor = value;
	}
	void Label::SetAlpha(BYTE value)
	{
		Window::SetAlpha(value);
		SetTextColor((static_cast<DWORD>(value) << 24) | (textColor & 0xFFFFFF));
	}
	void Label::SetColor(DWORD value)
	{
		Window::SetColor(value);
		SetTextColor((value & 0xFF000000) | (textColor & 0xFFFFFF));
	}
	void Label::Offset(LONG x, LONG y)
	{
		Window::Offset(x, y);
		for (LONG i = 0; i < static_cast<LONG>(displayedCharacters); ++i)
			::OffsetRect(&glyphRectangles[i], x, y);
	}
	float Label::SetDepth(float depth, float increment)
	{
		const float current = Window::SetDepth(depth, increment);
		for (DWORD i = 0; i < textVertexCount; ++i) textVertices[i].z = current;
		depth = increment * 0.25f + current;
		for (int i = 0; i < 9; ++i) fragments[i].SetDepth(depth, increment);
		return depth;
	}
	void Label::UpdateGeometry()
	{
		Window::UpdateGeometry();
		textGeometryDirty = 1;
		float sx, sy;
		g_Manager.GetScale(sx, sy);
		for (LONG i = 0; i < static_cast<LONG>(displayedCharacters); ++i)
		{
			textVertices[i * 6].x = static_cast<float>(glyphRectangles[i].left * sx - double(sx * 0.5f));
			textVertices[i * 6].y = static_cast<float>(glyphRectangles[i].top * sy - double(sy * 0.5f));
			textVertices[i * 6 + 1].x = static_cast<float>((glyphRectangles[i].right - 1) * sx + double(sx * 0.5f));
			textVertices[i * 6 + 1].y = textVertices[i * 6].y;
			textVertices[i * 6 + 2].x = textVertices[i * 6].x;
			textVertices[i * 6 + 2].y = static_cast<float>((glyphRectangles[i].bottom - 1) * sy + double(sy * 0.5f));
			textVertices[i * 6 + 3].x = textVertices[i * 6 + 2].x;
			textVertices[i * 6 + 3].y = textVertices[i * 6 + 2].y;
			textVertices[i * 6 + 4].x = textVertices[i * 6 + 1].x;
			textVertices[i * 6 + 4].y = textVertices[i * 6 + 1].y;
			textVertices[i * 6 + 5].x = textVertices[i * 6 + 1].x;
			textVertices[i * 6 + 5].y = textVertices[i * 6 + 2].y;
		}
	}
	void Label::Draw()
	{
		Window::Draw();
		if (!hidden && !text.empty() && static_cast<LONG>(displayedCharacters) > 0)
		{
			if (!g_Manager.packets->optimizationLevel)
				g_GfxSystem->QueueMeshInstances(&textInstance, viewport, 1);
			else g_Manager.packets->Submit(&textInstance, viewport, 3);
		}
	}
}
