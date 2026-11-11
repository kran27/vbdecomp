#include "gui/Label.hpp"
#include "gui/Manager.hpp"

namespace GUI
{
	LONG Label::GetTextAdvance(LONG startX) //4A89C0->4A8900->4A8200.
	{
		return GetTextAdvance(text,startX);
	}
	LONG Label::GetVisibleTextHeight() const { return textHeight; } //4A7760.
	void Label::SetScroll(LONG horizontal,LONG vertical) //4AA130.
	{
		horizontalScroll=horizontal; verticalScroll=vertical;
		RebuildText();
	}
}
