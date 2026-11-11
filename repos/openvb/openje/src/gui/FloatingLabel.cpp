#include "gui/FloatingLabel.hpp"

namespace GUI
{
	FloatingLabel::FloatingLabel() : Label(3), background(0)
	{
		kind = 0;
		birthTime = 0;
		duration = 0;
		fadeOut = 0;
		fadeIn = 0;
		speed = 0;
		entityID = 0;
		backgroundAlpha = 180;
	}
	FloatingLabel::~FloatingLabel() {}
	void FloatingLabel::MoveAtScreen(LONG x, LONG y, LONG width, LONG height, DWORD now)
	{
		// Original captures this subtraction before the text-width callback.
		LONG top = LONG(DWORD(y) - DWORD(height));
		LONG left = x;
		LONG alpha = 255;
		if (kind == 2) left = LONG(DWORD(left) - DWORD(width / 2));
		else if (kind == 1) left = LONG(DWORD(left) - DWORD(GetTextWidth() / 2));
		top = LONG(DWORD(top) - (now - birthTime) * speed / 1000);
		if (now < birthTime + fadeIn) alpha = static_cast<LONG>(255 * (now - birthTime) / fadeIn);
		if (now > birthTime + duration - fadeOut)
			alpha = static_cast<LONG>(255 * (birthTime + duration - now) / fadeOut);
		if (alpha < 0) alpha = 0;
		else if (alpha > 255) alpha = 255;
		SetAbsolutePosition(left, top);
		SetAlpha(static_cast<BYTE>(alpha));
		if (kind == 1) background.SetAlpha(static_cast<BYTE>(alpha * backgroundAlpha / 255));
		// Preserve four separate original expiration publications.
		if (left > 1024) duration = 0;
		if (LONG(DWORD(width) + DWORD(left)) < 0) duration = 0;
		if (top > 768) duration = 0;
		if (LONG(DWORD(height) + DWORD(top)) < 0) duration = 0;
	}
}
