#pragma once

#include "Types.hpp"
#include "NetworkRuntime.hpp"
#include <cstdlib>
#include <new>

namespace JE
{
	namespace Net
	{
		namespace BufferServices
		{
			void Free(void* pData);
		}

		class Buffer
		{
		public:
			struct Underflow : public Exception
			{
			public: Underflow() : Exception("Buffer underflow.") {}
			};
			struct Overflow : public Exception
			{
			public: Overflow() : Exception("Buffer overflow.") {}
			};
			Buffer();
			Buffer(UInt8* storage, UInt32 count)
				: capacity(0), field0x0C(0), field0x10(0), data(NULL)
			{
				// The native borrowed-storage path reads the incoming ownership byte.
				if (ownsData)
				{
					::operator delete[](data);
				}
				ownsData = 0;
				capacity = count;
				data = storage;
				field0x10 = 0;
				field0x0C = 0;
			}
			virtual ~Buffer() //45F060; actual inline ownership cleanup.
			{
				if (ownsData)
				{
					::operator delete[](data);
				}
				ownsData = 0;
				data = NULL;
			}
			UInt32 WriteByte(UInt8 value) //46CFB0; inline-visible in original packet callers.
			{
				if (capacity - field0x10 < 1) throw Overflow();
				data[field0x0C] = value;
				++field0x0C;
				if (field0x0C > field0x10) field0x10 = field0x0C;
				return field0x0C;
			}
			UInt32 WriteWord(UInt16 value); //588D30.
			UInt32 WriteSignedDword(Int32 value); //45F0B0, real signed SAR shifts.
			UInt32 WriteDword(UInt32 value); //45F130, little endian.
			Buffer& Write(const void* source, UInt32 count); //45F1B0.
			void ReadByte(UInt8& value) //45E4C0; writes output, no used return contract.
			{
				if (field0x10 - field0x0C < 1) throw Underflow();
				value = data[field0x0C];
				++field0x0C;
			}
			int ReadBool(UInt8& value); //45E460; normalized full-EAX result; cursor before output.
			void ReadWord(UInt16& value) //552930; return declaration is inferred.
			{
				if (field0x10 - field0x0C < 2) throw Underflow();
				value = static_cast<JE::Net::UInt16>(data[field0x0C]
					| (static_cast<JE::Net::UInt16>(data[field0x0C+1]) << 8));
				field0x0C += 2;
			}
			void ReadDword(UInt32& value) //45E500; actual inline-visible reader.
			{
				if (field0x10 - field0x0C < 4) throw Underflow();
				value = static_cast<UInt32>(data[field0x0C])
					| (static_cast<UInt32>(data[field0x0C+1]) << 8)
					| (static_cast<UInt32>(data[field0x0C+2]) << 16)
					| (static_cast<UInt32>(data[field0x0C+3]) << 24);
				field0x0C += 4;
			}
			void ReadFloat(float& value); //45E560; typed x87 load/store of the word.
			Buffer& Read(void* output,UInt32 count); //45E580.
			Buffer& ReadCString(char* output); //45E5E0, includes terminating NUL.
			Buffer& CopyFrom(Buffer& source, UInt32 count); //552BD0.

			UInt8 ownsData;
			UInt32 capacity;
			UInt32 field0x0C; //Current read/write cursor.
			UInt32 field0x10; //High water mark, not the cursor.
			UInt8* data;
		private:
			Buffer(const Buffer&);
			Buffer& operator=(const Buffer&);
		};

#if defined(_MSC_VER) && _MSC_VER == 1300
		typedef char BufferUnderflowOriginalSize08[(sizeof(Buffer::Underflow) == 0x08) ? 1 : -1];
		typedef char BufferOverflowOriginalSize08[(sizeof(Buffer::Overflow) == 0x08) ? 1 : -1];
#endif

	}
}
