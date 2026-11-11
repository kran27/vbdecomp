#include "TextureMask.hpp"

namespace GUI
{
	bool TextureMask::Test(LONG x, LONG y)
	{
		BYTE* buffer = bits;
		if (!buffer) return true;
		LONG index = static_cast<LONG>(static_cast<DWORD>(rowWidth)
			* static_cast<DWORD>(y) + static_cast<DWORD>(x));
		LONG remainder = index % 8;
		BYTE mask = static_cast<BYTE>(1U << (static_cast<DWORD>(remainder) & 31));
		// Even a negative remainder (and zero byte mask) still reads the buffer.
		return (buffer[index / 8] & mask) != 0;
	}
}
