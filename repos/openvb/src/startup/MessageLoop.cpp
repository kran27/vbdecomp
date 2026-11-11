// Copyright 2024 OpenJE
#include "MessageLoop.hpp"
#include "Window.hpp"

namespace F3
{
	// 0x56AFF0: shared OS-message API, called by the window callback.
	void Quit()
	{
		PostQuitMessage(1);
	}

	// Original globals: previous reading at 0x70C2BC, accumulated time at 0x70C2B8.
	unsigned long g_nSystemTimeMS = 0;
	unsigned long g_nElapsedTimeMS = 0;

	DWORD GetElapsedTimeMS() //56AF50 reads accumulated70C2B8; it does not poll WinMM.
	{
		return g_nElapsedTimeMS;
	}

	// 0x56AF60.
	bool ProcessMessagesAndUpdateTime()
	{
		MSG message;
		while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
			{
				return false;
			}
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		const DWORD nCurrentTime = timeGetTime();
		g_nElapsedTimeMS += nCurrentTime - g_nSystemTimeMS;
		g_nSystemTimeMS = nCurrentTime;
		return true;
	}
}
