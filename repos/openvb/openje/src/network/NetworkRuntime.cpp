#include "NetworkRuntime.hpp"

// Original startup calls use the WinSock import-library thunk.
#define WINSOCK_API_LINKAGE
#include <winsock2.h>
#include <mmsystem.h>

namespace JE
{
	namespace Net
	{
		JE::Net::UInt32 g_RuntimeReferences = 0;

		void InitializeRuntime()
		{
			// The original compares the incremented DWORD unsigned against one.
			// Only old counts zero and 0xFFFFFFFF enter the initialization block.
			++g_RuntimeReferences;
			if (g_RuntimeReferences <= 1)
			{
				WSADATA data;
				if (RuntimeServices::Startup(0x0101, &data))
					throw Exception("Unable to initialize Winsock.");
				if (RuntimeServices::BeginPeriod(1))
					throw Exception("Unable to set timer resolution.");
			}
		}

		void ShutdownRuntime()
		{
			if (g_RuntimeReferences)
			{
				--g_RuntimeReferences;
				if (!g_RuntimeReferences)
				{
					RuntimeServices::EndPeriod(1);
					RuntimeServices::Cleanup();
				}
			}
		}

		namespace RuntimeServices
		{
			int Startup(JE::Net::UInt16 version, WSAData* data) { return WSAStartup(version, data); }
			unsigned int BeginPeriod(unsigned int period) { return timeBeginPeriod(period); }
			unsigned int EndPeriod(unsigned int period) { return timeEndPeriod(period); }
			int Cleanup() { return WSACleanup(); }
		}
	}
}
