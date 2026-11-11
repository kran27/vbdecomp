#include "Buffer.hpp"
#include <cstring>

namespace JE
{
	namespace Net
	{
		void Buffer::ReadFloat(float& value)
		{
			union { JE::Net::UInt32 word; float number; } converted;
			ReadDword(converted.word);
			value = converted.number;
		}
		Buffer& Buffer::Read(void* output,JE::Net::UInt32 count)
		{
			if (count>field0x10-field0x0C) throw Underflow();
			std::memcpy(output,data+field0x0C,count);
			field0x0C+=count;
			return *this;
		}
		Buffer& Buffer::ReadCString(char* output)
		{
			//45E5ED computes strlen before the packet-length test. Preserve the
			//original terminated-input domain rather than inventing a new parser.
			JE::Net::UInt32 count=JE::Net::UInt32(std::strlen(reinterpret_cast<const char*>(data+field0x0C))+1);
			if (count>field0x10-field0x0C) throw Underflow();
			return Read(output,count);
		}
	}
}
