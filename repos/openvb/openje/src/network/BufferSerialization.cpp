#include "Buffer.hpp"
#include <cstring>

namespace JE
{
	namespace Net
	{
		JE::Net::UInt32 Buffer::WriteWord(JE::Net::UInt16 value)
		{
			if (capacity - field0x10 < 2) throw Overflow();
			data[field0x0C] = static_cast<JE::Net::UInt8>(value);
			data[field0x0C+1] = static_cast<JE::Net::UInt8>(value >> 8);
			field0x0C += 2;
			if (field0x0C > field0x10) field0x10 = field0x0C;
			return field0x0C;
		}

		JE::Net::UInt32 Buffer::WriteSignedDword(JE::Net::Int32 value)
		{
			if (capacity - field0x10 < 4) throw Overflow();
			data[field0x0C] = static_cast<JE::Net::UInt8>(value);
			data[field0x0C+1] = static_cast<JE::Net::UInt8>(value >> 8);
			data[field0x0C+2] = static_cast<JE::Net::UInt8>(value >> 16);
			data[field0x0C+3] = static_cast<JE::Net::UInt8>(value >> 24);
			field0x0C += 4;
			if (field0x0C > field0x10) field0x10 = field0x0C;
			return field0x0C;
		}

		JE::Net::UInt32 Buffer::WriteDword(JE::Net::UInt32 value)
		{
			//Original capacity check uses the high water mark even for overwrites.
			if (capacity - field0x10 < 4) throw Overflow();
			data[field0x0C] = static_cast<JE::Net::UInt8>(value);
			data[field0x0C+1] = static_cast<JE::Net::UInt8>(value >> 8);
			data[field0x0C+2] = static_cast<JE::Net::UInt8>(value >> 16);
			data[field0x0C+3] = static_cast<JE::Net::UInt8>(value >> 24);
			field0x0C += 4;
			if (field0x0C > field0x10) field0x10 = field0x0C;
			return field0x0C;
		}

		Buffer& Buffer::Write(const void* source, JE::Net::UInt32 count)
		{
			if (count > capacity - field0x10) throw Overflow();
			std::memcpy(data+field0x0C, source, count);
			field0x0C += count;
			if (field0x0C > field0x10) field0x10 = field0x0C;
			return *this;
		}



		Buffer& Buffer::CopyFrom(Buffer& source, JE::Net::UInt32 count)
		{
			if (count > capacity - field0x10) throw Overflow();
			if (count > source.field0x10 - source.field0x0C) throw Underflow();
			std::memcpy(data+field0x0C, source.data+source.field0x0C, count);
			field0x0C += count;
			if (field0x0C > field0x10) field0x10 = field0x0C;
			source.field0x0C += count;
			return *this;
		}
	}
}
