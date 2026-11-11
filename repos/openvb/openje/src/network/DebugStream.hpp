#pragma once

#include "Types.hpp"
#include "Logger.hpp"

namespace JE { namespace Net
{
	// Original net::debug_stream member, Base+0C. Its one virtual slot is
	// the destructor; flags/logger are consumed at +04/+08 by5B8C40.
	class DebugStream
	{
	public:
		DebugStream() : debugFlags(0), logger(0) {}
		virtual ~DebugStream() {} //59D4C0, destructor's vfptr store.
		void Print(UInt32 mask,const char* format,...); //5B8C40; original member varargs ABI.
		UInt32 debugFlags;
		Logger logger;
	};
} }
