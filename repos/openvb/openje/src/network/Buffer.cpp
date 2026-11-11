#include "Buffer.hpp"

#include <cstdlib>

namespace JE
{
	namespace Net
	{
		Buffer::Buffer() : ownsData(0), capacity(0), field0x0C(0), field0x10(0), data(NULL)
		{
		}

		int Buffer::ReadBool(UInt8& value) //45E460, normalized-byte output; original type spelling unproved.
		{
			if (field0x10-field0x0C<1) throw Underflow();
			UInt8 byte=data[field0x0C];
			++field0x0C;
			if (byte)
			{
				value=1;
				return 1;
			}
			else
			{
				value=0;
				return 0;
			}
		}

		namespace BufferServices
		{
			void Free(void* pData) { std::free(pData); }
		}
	}
}
