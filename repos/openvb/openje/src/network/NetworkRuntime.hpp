#pragma once

#include "Types.hpp"
#include <exception>
#include <cstddef>

struct WSAData; // Actual Windows SDK tag; defined by winsock2.h.

namespace JE
{
	namespace Net
	{
		//Original .?AUexception@net@@ has no base and owns only a borrowed
		//message pointer. Its sole virtual is destruction, not what().
		struct Exception
		{
			explicit Exception(const char* text) : message(text) {}
			virtual ~Exception() {} //45E3E0; no message release.
			//Existing workspace startup's convenience view; no original method
			//identity or virtual-slot credit is assigned to this accessor.
			const char* what() const { return message; }
			const char* message; //04; shallow-copy/borrowed lifetime.
		};

#if defined(_MSC_VER) && _MSC_VER == 1300
		typedef char NetExceptionOriginalSize08[(sizeof(Exception) == 0x08) ? 1 : -1];
		typedef char NetExceptionOriginalMessage04[(offsetof(Exception,message) == 0x04) ? 1 : -1];
#endif

		extern UInt32 g_RuntimeReferences; // Original 0x70C6C0.
		void InitializeRuntime(); // 0x5A2A10.
		void ShutdownRuntime(); // Shared cleanup chunk 0x5A2AB0 in 0x543730.

		namespace RuntimeServices
		{
			int Startup(UInt16 version, WSAData* data);
			unsigned int BeginPeriod(unsigned int period);
			unsigned int EndPeriod(unsigned int period);
			int Cleanup();
		}
	}
}
