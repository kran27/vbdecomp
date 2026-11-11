#pragma once

#include <windows.h>

namespace GUI
{
	// Original six-DWORD event transport used by 4B3040/4B3500/4B37F0.
	// The type-zero Update producer initializes only type, not the other words.
	struct Event
	{
		int type;
		LONG x, y;
		DWORD buttons;
		LONG wheel;
		DWORD modifiers;
	};
}
