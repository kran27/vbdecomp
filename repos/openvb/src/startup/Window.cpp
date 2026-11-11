#include "Window.hpp"
#include "SaveDirectory.hpp"
#include "ConfigurationObject.hpp"
#include "diagnostics/DebugOutput.hpp"

#include <cstdlib>
#include <cstring>


namespace F3
{
	HINSTANCE g_instance;
	HWND g_window;

	namespace Display
	{
		static LONG Subtract32(LONG nLeft, LONG nRight)
		{
			return static_cast<LONG>(static_cast<DWORD>(nLeft) - static_cast<DWORD>(nRight));
		}

		static LONG Add32(LONG nLeft, LONG nRight)
		{
			return static_cast<LONG>(static_cast<DWORD>(nLeft) + static_cast<DWORD>(nRight));
		}

		// Original VA 0x56B470. Valid class-name input fits 256 bytes including NUL.
		void CreateGameWindow(int nCmdShow)
		{
			char szClassName[256];
			std::strcpy(szClassName, GetSaveDirectoryRelative());
			WNDCLASSA windowClass;
			windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
			windowClass.lpfnWndProc = WinProc;
			windowClass.cbClsExtra = 0;
			windowClass.cbWndExtra = 0;
			windowClass.hInstance = g_instance;
			windowClass.hIcon = static_cast<HICON>(
				LoadImageA(g_instance, szClassName, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
			windowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
			windowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
			windowClass.lpszMenuName = NULL;
			windowClass.lpszClassName = szClassName;
			if (!RegisterClassA(&windowClass))
			{
				JE::FatalError("Unable to register window class.");
			}

			int nWidth = 800;
			int nHeight = 600;
			JE::Configuration::Settings::iterator section = g_SettingsObject.find("Graphics");
			if (section != g_SettingsObject.end())
			{
				JE::Configuration::Values& values = section->second;
				JE::Configuration::Values::const_iterator value = values.find("width");
				if (value != values.end())
				{
					nWidth = std::atol(value->second.c_str());
				}
				value = values.find("height");
				if (value != values.end())
				{
					nHeight = std::atol(value->second.c_str());
				}
			}

			RECT rect;
			SetRect(&rect, 0, 0, nWidth, nHeight);
			const DWORD dwStyle = WS_VISIBLE | WS_CAPTION | WS_SYSMENU;
			AdjustWindowRect(&rect, dwStyle, FALSE);
			// Original captures sizes before either metric service, then reloads
			// rectangle coordinates after each call for signed centering.
			const LONG nOuterHeight = Subtract32(rect.bottom, rect.top);
			const LONG nOuterWidth = Subtract32(rect.right, rect.left);
			const int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
			const LONG nY = Add32(nScreenHeight, Subtract32(rect.top, rect.bottom)) / 2;
			const int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
			const LONG nX = Add32(nScreenWidth, Subtract32(rect.left, rect.right)) / 2;
			HWND window = CreateWindowExA(0, szClassName, szClassName, dwStyle,
				nX, nY, nOuterWidth, nOuterHeight, NULL, NULL, NULL, NULL);
			g_window = window;
			if (!window)
			{
				JE::FatalError("Could not create main window.");
			}
			ShowWindow(window, nCmdShow);
			UpdateWindow(g_window);
		}

		// Original VA 0x56B1A0. No null guard, reset or error branch.
		void DestroyGameWindow()
		{
			DestroyWindow(g_window);
		}
	}
}
