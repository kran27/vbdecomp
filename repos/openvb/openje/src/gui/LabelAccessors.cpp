#include "gui/Label.hpp"

namespace GUI
{
	void Label::SetFrameVisible(BYTE value) { SetTextFrame(value); } //4A7B20, actual virtual+178 tail dispatch.

	void Label::AppendText(const std::string& value) //4AA280.
	{
		text.append(value);
		RebuildText();
	}

	void Label::UnusedTextOperation(DWORD) {} //Original shared5A9320 RET4, not an unfinished body.

	void Label::GetText(std::string& output) { output = text; } //4A8A90.
	DWORD Label::GetStringReference(DWORD& output) const { return output = stringReference; } //4A7740.

	void Label::AppendCharacter(char value) //4AA2B0.
	{
		text.append(1, value);
		RebuildText();
	}

	void Label::SetFontSize(LONG value) { fontSize = value; RebuildText(); } //4AA310.
	LONG Label::GetMaximumRowWidth() const { return maximumRowWidth; } //4A7790.
	DWORD Label::GetTextColor() const { return textColor; } //4A7810.

	void Label::SetPadding(LONG left, LONG right, LONG top, LONG bottom) //4AA330.
	{
		paddingLeft = left;
		paddingRight = right;
		paddingTop = top;
		paddingBottom = bottom;
		RebuildText();
	}

	void Label::GetPadding(LONG& left, LONG& right, LONG& top, LONG& bottom) const //4A7820.
	{
		left = paddingLeft;
		right = paddingRight;
		top = paddingTop;
		bottom = paddingBottom;
	}

	BYTE Label::GetTextFrame() const { return textFrame; } //4A7890.
	BYTE Label::GetWrap() const { return wrap; } //4A78A0.
	DWORD Label::GetAlignment() const { return alignment; } //4A78B0.
	DWORD Label::GetVerticalAlignment() const { return verticalAlignment; } //4A78C0.
	LONG Label::GetScroll(LONG& horizontal, LONG& vertical) const //4A7D40.
	{
		horizontal = horizontalScroll;
		return vertical = verticalScroll;
	}
	void Label::SetTabWidth(LONG value) { tabWidth = value; } //4588A0.
	LONG Label::GetTabWidth() const { return tabWidth; } //4588B0.
}
