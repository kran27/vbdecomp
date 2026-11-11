#include "Window.hpp"
#include "gui/Manager.hpp"


namespace F3
{

	namespace Display
	{
		// Original VA 0x56B1B0. Capture release precedes default message handling.
		LRESULT CALLBACK WinProc(HWND hWindow, UINT nMessage, WPARAM wParam, LPARAM lParam)
		{
			if (hWindow != g_window)
			{
				return DefWindowProcA(hWindow, nMessage, wParam, lParam);
			}
			if (nMessage == WM_CLOSE)
			{
				Quit();
				return 0;
			}
			if (nMessage == WM_ACTIVATEAPP && !wParam)
			{
				::GUI::g_Manager.SetCapture(NULL);
			}
			return DefWindowProcA(hWindow, nMessage, wParam, lParam);
		}
	}
}
