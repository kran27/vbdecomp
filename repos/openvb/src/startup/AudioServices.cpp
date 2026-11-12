#include "audio/Audio.hpp"
#include "audio/SoundSystem.hpp"
#include <windows.h>
#include <process.h>

namespace JE
{
	namespace AudioServices
	{
		unsigned __stdcall Worker(void*) //5909A0.
		{
			while (g_WorkerRunning)
			{
				::Sleep(100);
				AudioBackend::UpdateBackend();
			}
			::_endthreadex(0);
			return 0;
		}

		JE::Net::UInt8 g_WorkerRunning = 1; // Original byte 0x6FF2ED is one.
	}
}
