#include "Win32Input.hpp"

#include "startup/Window.hpp"
#include "startup/ConfigurationObject.hpp"

#include <cstdio>

namespace JE
{
	//The original callers retain cdecl cleanup and continuation after48CC40.
	void __cdecl FatalError(const char* format, ...);
}

namespace JE
{
	namespace Input
	{
		HHOOK g_hook = NULL;
		Win32Input* g_pInput = NULL;
		DWORD g_modifiers = 0;
		LONG g_lastMouseX = 0, g_lastMouseY = 0;

		Win32Input::Win32Input()
		{
			if (g_pInput) Win32InputServices::Fatal("Input object already exists.");
			g_pInput = this;
			const DWORD thread = Win32InputServices::ThreadId();
			g_hook = Win32InputServices::InstallHook(WH_GETMESSAGE, MessageHook, NULL, thread);
			if (!g_hook) Win32InputServices::Fatal("Unable to set window procedure hook.");
			for (DWORD scan = 0; scan < sizeof(keyNames) / sizeof(keyNames[0]); ++scan)
			{
				char name[256];
				if (!Win32InputServices::KeyNameText(static_cast<LONG>(scan << 16), name, sizeof(name)))
					std::sprintf(name, "0x%x", scan);
				keyNames[scan].assign(name);
			}
		}

		Win32Input::~Win32Input()
		{
			if (g_hook)
			{
				if (!Win32InputServices::RemoveHook(g_hook))
					Win32InputServices::Fatal("Unable to release window procedure hook.");
				g_hook = NULL;
			}
			g_pInput = NULL;
		}

		void Win32Input::Initialize()
		{
			//5B49E0 performs the section lookup and destroys its temporary key.
			//It neither inserts an absent section nor reads any configuration keys.
			Configuration::Settings& settings = *F3::GetSettingsObject();
			settings.find(std::string("Input"));
		}

		DWORD Win32Input::FindKey(const char* name) const
		{
			const std::string key(name);
			for (DWORD scan = 0; scan < 512; ++scan)
				if (keyNames[scan] == key) return scan;
			return 0;
		}

		const char* Win32Input::KeyName(DWORD scanCode) const
		{
			return keyNames[scanCode].c_str();
		}

		LRESULT CALLBACK MessageHook(int code, WPARAM wParam, LPARAM lParam)
		{
			if (code || !wParam) goto passMessage;
			{
				const HWND window = F3::g_window;
				const MSG& message = *reinterpret_cast<const MSG*>(lParam);
				if (message.hwnd != window) goto passMessage;
				InputEvent event; // Original leaves inactive fields and padding unwritten.
				WPARAM buttons;
				switch (message.message)
				{
				case WM_KEYDOWN:
					event.category = 0;
					event.subtype = 0;
					event.x = HIWORD(message.lParam) & 0x1FF;
					break;
				case WM_KEYUP:
					event.category = 0;
					event.subtype = 1;
					event.x = HIWORD(message.lParam) & 0x1FF;
					break;
				case WM_CHAR:
					event.category = 1;
					event.character = static_cast<BYTE>(message.wParam);
					break;
				case WM_MOUSEMOVE:
				{
					const LONG x = static_cast<SHORT>(LOWORD(message.lParam));
					const LONG y = static_cast<SHORT>(HIWORD(message.lParam));
					const LONG deltaX = static_cast<LONG>(DWORD(x) - DWORD(g_lastMouseX));
					event.x = x;
					g_lastMouseX = x;
					//The left byte is captured before publishing the new Y.
					const BYTE left = static_cast<BYTE>(message.wParam & MK_LBUTTON);
					event.deltaX = deltaX;
					const LONG deltaY = static_cast<LONG>(DWORD(y) - DWORD(g_lastMouseY));
					event.y = y;
					g_lastMouseY = y;
					event.left = left;
					buttons = message.wParam;
					event.subtype = 0;
					event.deltaY = deltaY;
					event.button = 0;
					event.middle = (buttons & MK_MBUTTON) != 0;
					goto finishMouse;
				}
				case WM_LBUTTONDOWN:
					event.subtype = 1;
					event.button = 0;
					event.x = static_cast<SHORT>(LOWORD(message.lParam));
					event.y = static_cast<SHORT>(HIWORD(message.lParam));
					event.left = (message.wParam & MK_LBUTTON) != 0;
					buttons = message.wParam;
					event.middle = (buttons & MK_MBUTTON) != 0;
					goto finishStillMouse;
				case WM_LBUTTONUP:
					event.subtype = 2;
					event.button = 0;
					event.x = static_cast<SHORT>(LOWORD(message.lParam));
					event.y = static_cast<SHORT>(HIWORD(message.lParam));
					event.left = (message.wParam & MK_LBUTTON) != 0;
					buttons = message.wParam;
					event.middle = (buttons & MK_MBUTTON) != 0;
					goto finishStillMouse;
				case WM_LBUTTONDBLCLK:
					event.subtype = 3;
					event.button = 0;
					event.x = static_cast<SHORT>(LOWORD(message.lParam));
					event.y = static_cast<SHORT>(HIWORD(message.lParam));
					event.left = (message.wParam & MK_LBUTTON) != 0;
					buttons = message.wParam;
					event.middle = (buttons & MK_MBUTTON) != 0;
					goto finishStillMouse;
				case WM_MBUTTONDOWN:
				{
					const LONG x = static_cast<SHORT>(LOWORD(message.lParam));
					const LONG y = static_cast<SHORT>(HIWORD(message.lParam));
					event.subtype = 1;
					event.button = 1;
					const BYTE left = static_cast<BYTE>(message.wParam & MK_LBUTTON);
					event.x = x;
					event.left = left;
					buttons = message.wParam;
					event.y = y;
					event.middle = (buttons & MK_MBUTTON) != 0;
					goto finishStillMouse;
				}
				case WM_MBUTTONUP:
					event.subtype = 2;
					event.button = 1;
					event.x = static_cast<SHORT>(LOWORD(message.lParam));
					event.y = static_cast<SHORT>(HIWORD(message.lParam));
					event.left = (message.wParam & MK_LBUTTON) != 0;
					buttons = message.wParam;
					event.middle = (buttons & MK_MBUTTON) != 0;
					goto finishStillMouse;
				case WM_MBUTTONDBLCLK:
					event.subtype = 3;
					event.button = 1;
					event.x = static_cast<SHORT>(LOWORD(message.lParam));
					event.y = static_cast<SHORT>(HIWORD(message.lParam));
					event.left = (message.wParam & MK_LBUTTON) != 0;
					buttons = message.wParam;
					event.middle = (buttons & MK_MBUTTON) != 0;
					goto finishStillMouse;
				case WM_RBUTTONDOWN:
					event.subtype = 1;
					event.button = 2;
					event.x = static_cast<SHORT>(LOWORD(message.lParam));
					event.y = static_cast<SHORT>(HIWORD(message.lParam));
					event.left = (message.wParam & MK_LBUTTON) != 0;
					buttons = message.wParam;
					event.middle = (buttons & MK_MBUTTON) != 0;
					goto finishStillMouse;
				case WM_RBUTTONUP:
					event.subtype = 2;
					event.button = 2;
					event.x = static_cast<SHORT>(LOWORD(message.lParam));
					event.y = static_cast<SHORT>(HIWORD(message.lParam));
					event.left = (message.wParam & MK_LBUTTON) != 0;
					buttons = message.wParam;
					event.middle = (buttons & MK_MBUTTON) != 0;
					goto finishStillMouse;
				case WM_RBUTTONDBLCLK:
				{
					const BYTE left = static_cast<BYTE>(message.wParam);
					const LONG x = static_cast<SHORT>(LOWORD(message.lParam));
					const LONG y = static_cast<SHORT>(HIWORD(message.lParam));
					event.x = x;
					event.left = left & MK_LBUTTON;
					buttons = message.wParam;
					event.subtype = 3;
					event.y = y;
					event.button = 2;
					event.middle = (buttons & MK_MBUTTON) != 0;
					goto finishStillMouse;
				}
				case WM_MOUSEWHEEL:
				{
					POINT point = {static_cast<SHORT>(LOWORD(message.lParam)),
						static_cast<SHORT>(HIWORD(message.lParam))};
					event.category = 2;
					event.subtype = 4;
					ScreenToClient(window, &point);
					event.x = point.x;
					event.y = point.y;
					buttons = message.wParam;
					event.wheel = static_cast<SHORT>(HIWORD(buttons)) / WHEEL_DELTA;
					event.deltaX = 0;
					event.deltaY = 0;
					event.button = 0;
					event.left = (buttons & MK_LBUTTON) != 0;
					event.middle = (buttons & MK_MBUTTON) != 0;
					event.right = (buttons & MK_RBUTTON) != 0;
					break;
				}
				default:
					goto passMessage;
				}
				goto enqueue;
				finishStillMouse:
				event.deltaX = 0;
				event.deltaY = 0;
				finishMouse:
				event.category = 2;
				event.wheel = 0;
				event.right = (buttons & MK_RBUTTON) != 0;
				enqueue:
				g_modifiers = 0;
				if (GetAsyncKeyState(VK_SHIFT) & 0x8000) g_modifiers |= 2;
				if (GetAsyncKeyState(VK_CONTROL) & 0x8000) g_modifiers |= 1;
				if (GetAsyncKeyState(VK_MENU) & 0x8000) g_modifiers |= 4;
				event.modifiers = g_modifiers;
				g_pInput->PushEvent(event);
			}
		passMessage:
			return CallNextHookEx(g_hook, code, wParam, lParam);
		}

		namespace Win32InputServices
		{
			DWORD ThreadId() { return GetCurrentThreadId(); }
			HHOOK InstallHook(int kind, HOOKPROC procedure, HINSTANCE instance, DWORD thread)
			{ return SetWindowsHookExA(kind, procedure, instance, thread); }
			BOOL RemoveHook(HHOOK hook) { return UnhookWindowsHookEx(hook); }
			int KeyNameText(LONG scanCode, char* output, int size)
			{ return GetKeyNameTextA(scanCode, output, size); }
			SHORT AsyncKey(int key) { return GetAsyncKeyState(key); }
			BOOL ClientPoint(HWND window, POINT* point) { return ScreenToClient(window, point); }
			LRESULT NextHook(HHOOK hook, int code, WPARAM wParam, LPARAM lParam)
			{ return CallNextHookEx(hook, code, wParam, lParam); }
			void Fatal(const char* message) { JE::FatalError(message); }
		}
	}
}
