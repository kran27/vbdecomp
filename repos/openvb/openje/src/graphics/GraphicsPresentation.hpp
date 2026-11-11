#pragma once

#include <windows.h>

class Gfx_System_Interface;
namespace GUI { class Manager; }

namespace JE
{
	//5811C0 with original outer Client/Graphics/clock owners supplied explicitly.
	void PresentGraphicsFrame(DWORD& previousClock, const BYTE& clientFlag10A,
		::GUI::Manager* manager, Gfx_System_Interface*& backend, DWORD (*clock)());
}
