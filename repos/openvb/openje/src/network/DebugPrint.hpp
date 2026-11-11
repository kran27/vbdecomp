#pragma once

#include "Types.hpp"
#include "Logger.hpp"

namespace JE
{
	namespace Net
	{
		//5B8C40 source factor: original debug_stream owns flags/logger, while the
		//existing normal owners expose them directly. Callback receives one fully
		//formatted string; MSVC supplies va_list/vsprintf and exception transport.
		void DebugPrint(JE::Net::UInt32 flags, Logger logger, JE::Net::UInt32 mask, const char* format, ...);
	}
}
