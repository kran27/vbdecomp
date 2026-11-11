#pragma once

#include "Input.hpp"
#include <string>
#include "configuration/ConfigurationTypes.hpp"

namespace JE
{
	namespace Input
	{
		class Win32Input : public Input
		{
		public:
			Win32Input(); //5B4770: actual hook/key-name construction policy.
			virtual ~Win32Input(); //5B46B0.
			virtual const char* KeyName(DWORD scanCode) const; //5B4690,domain<512.
			virtual DWORD FindKey(const char* name) const; //5B48B0, exact spelling.
			virtual void Initialize(); //5B49E0, main-settings Input lookup; no field changes.

			// Actual library objects own the event queue and key-name strings.
			std::string keyNames[512];
		private:
			Win32Input(const Win32Input&);
			Win32Input& operator=(const Win32Input&);
		};
		#if defined(_MSC_VER) && _MSC_VER == 1300
		typedef char Win32InputNativeSizeRequired[sizeof(Win32Input) == 0x3818 ? 1 : -1];
		typedef char Win32InputNativeNamesRequired[offsetof(Win32Input, keyNames) == 24 ? 1 : -1];
		#endif

		extern Win32Input* g_pInput; //70C958; publication precedes hook installation.
		extern HHOOK g_hook; //70C954.
		extern DWORD g_modifiers; //70C95C.
		extern LONG g_lastMouseX, g_lastMouseY; //70C960/64.
		LRESULT CALLBACK MessageHook(int code, WPARAM wParam, LPARAM lParam); //5B4300.

		namespace Win32InputServices
		{
			HWND Window(); //Required binding to the actual canonical F3 HWND.
			const Configuration::Settings& MainSettings(); //Actual fixed56AF30 owner, distinct from engine sys.ini.
			DWORD ThreadId();
			HHOOK InstallHook(int kind, HOOKPROC procedure, HINSTANCE instance, DWORD thread);
			BOOL RemoveHook(HHOOK hook);
			int KeyNameText(LONG scanCode, char* output, int size);
			SHORT AsyncKey(int key);
			BOOL ClientPoint(HWND window, POINT* point);
			LRESULT NextHook(HHOOK hook, int code, WPARAM wParam, LPARAM lParam);
			void Fatal(const char* message);
		}
	}
}
