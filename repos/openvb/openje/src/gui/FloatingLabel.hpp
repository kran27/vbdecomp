#pragma once

#include "gui/Label.hpp"

namespace GUI
{
	class FloatingLabel : public Label
	{
	public:
		FloatingLabel(); //58FA00.
		virtual ~FloatingLabel(); //58FA90.
		virtual void MoveAtScreen(LONG x, LONG y, LONG width, LONG height, DWORD now); //58FAF0.
		DWORD kind, birthTime, duration, fadeOut, fadeIn, speed, entityID;
		Window background;
		BYTE backgroundAlpha;
	};
}
