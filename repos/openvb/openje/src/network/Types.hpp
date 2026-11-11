#pragma once

#include <climits>

namespace JE
{
	namespace Net
	{
		typedef unsigned char UInt8;
		typedef unsigned short UInt16;
		typedef unsigned int UInt32;
		typedef unsigned __int64 UInt64;
		typedef signed char Int8;
		typedef short Int16;
		typedef int Int32;
		typedef __int64 Int64;

		typedef char IntegerWidthsRequired[(CHAR_BIT == 8 && sizeof(UInt8) == 1
			&& sizeof(UInt16) == 2 && sizeof(UInt32) == 4 && sizeof(UInt64) == 8
			&& sizeof(Int8) == 1 && sizeof(Int16) == 2 && sizeof(Int32) == 4
			&& sizeof(Int64) == 8) ? 1 : -1];
	}
}
