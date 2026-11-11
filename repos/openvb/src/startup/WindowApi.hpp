// Test-only replacements of Win32 calls. Production includes/calls the real SDK.
#pragma once
#include <windows.h>

// Avoid the SDK's generic-name macros in this test namespace.
#undef LoadImage
#undef LoadCursor
#undef RegisterClass
#undef CreateWindow

namespace F3
{
	namespace Display
	{
		namespace WindowApi
		{
			HANDLE WINAPI LoadImage(HINSTANCE, LPCSTR, UINT, int, int, UINT);
			HCURSOR WINAPI LoadCursor(HINSTANCE, LPCSTR);
			HGDIOBJ WINAPI StockObject(int);
			ATOM WINAPI RegisterClass(const WNDCLASSA*);
			BOOL WINAPI SetRectangle(LPRECT, int, int, int, int);
			BOOL WINAPI AdjustRectangle(LPRECT, DWORD, BOOL);
			int WINAPI Metrics(int);
			HWND WINAPI CreateWindow(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int,
				HWND, HMENU, HINSTANCE, LPVOID);
			BOOL WINAPI Show(HWND, int);
			BOOL WINAPI Update(HWND);
			BOOL WINAPI Destroy(HWND);
		}
	}
}
