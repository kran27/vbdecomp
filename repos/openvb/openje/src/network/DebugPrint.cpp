#include "DebugPrint.hpp"
#include "DebugStream.hpp"
#include "NetworkRuntime.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace JE
{
	namespace Net
	{
		void DebugStream::Print(JE::Net::UInt32 mask,const char* format,...)
		{
			if (!logger || !(debugFlags & mask)) return;
			char text[1024];
			va_list args;
			va_start(args,format);
			std::vsprintf(text,format,args);
			va_end(args);
			if (std::strlen(text)>1024) throw Exception("Memory overwrite in debug_stream::printf()");
			logger(text);
		}

		void DebugPrint(JE::Net::UInt32 flags, Logger logger, JE::Net::UInt32 mask, const char* format, ...)
		{
			if (!logger || !(flags & mask)) return;
			char text[1024];
			va_list args;
			va_start(args, format);
			std::vsprintf(text, format, args);
			va_end(args);
			//Original checks only after formatting; no stronger bounded-write
			//contract is asserted for oversized diagnostic strings.
			if (std::strlen(text) > 1024) throw Exception("Memory overwrite in debug_stream::printf()");
			logger(text);
		}
	}
}
