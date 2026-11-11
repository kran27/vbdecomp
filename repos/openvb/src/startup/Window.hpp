#pragma once

#include <windows.h>

namespace F3
{
	// One owner shared by WinMain, window creation, callback and destruction.
	extern HINSTANCE g_instance; // Original 0x70BFA0.
	extern HWND g_window; // Original 0x70C0AC.
	void Quit();

	namespace Display
	{
		void CreateGameWindow(int nCmdShow);
		void DestroyGameWindow();
		LRESULT CALLBACK WinProc(HWND hWindow, UINT nMessage, WPARAM wParam, LPARAM lParam);
	}
}
