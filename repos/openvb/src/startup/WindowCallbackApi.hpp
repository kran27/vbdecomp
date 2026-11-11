// Test-only callback services; production calls Win32 directly.
#pragma once
#include <windows.h>

namespace F3
{
	namespace Display
	{
		namespace WindowCallbackApi
		{
			LRESULT WINAPI DefaultProc(HWND, UINT, WPARAM, LPARAM);
			void WINAPI QuitMessage(int);
		}
	}
}
