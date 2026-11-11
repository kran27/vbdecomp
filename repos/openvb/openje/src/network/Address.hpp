#pragma once

#include "NetworkRuntime.hpp"
#include "Types.hpp"
#include <cstddef>

struct sockaddr_in;
struct hostent;

namespace JE
{
	namespace Net
	{
		struct AddressException : public Exception
		{
		public:
			explicit AddressException(const char* message) : Exception(message) {}
		};

		class Address
		{
		public:
			Address() //5A07B0; also inline-visible in real packet initialization.
			{
				bytes[3] = 0;
				bytes[2] = 0;
				bytes[1] = 0;
				bytes[0] = 0;
				port = 0;
			}
			Address(UInt8 first, UInt8 second, UInt8 third, UInt8 fourth, UInt16 value)
			{
				bytes[0] = first; bytes[1] = second; bytes[2] = third; bytes[3] = fourth;
				port = value;
			}
			virtual ~Address() {}
			Address& operator=(const Address& source); //5AC9E0; actual six-byte address assignment.
			char* Format(char* output,UInt32 capacity,UInt8 numericOnly) const; //5ACAF0; always includes port.
			UInt8 ToSockaddr(sockaddr_in& output) const; //5ACA10; leaves sin_zero untouched, returns third octet.
			void Resolve(const char* name); // 0x5ACC00.
			unsigned char Assign(const sockaddr_in& address); // 0x5ACA50; incidental AL is third octet.
			int Compare(const Address& address, UInt8 includePort) const; //5ACA90, normalized BOOL.

			UInt8 bytes[4];
			UInt16 port;
		};

		namespace AddressServices
		{
			char* Allocate(std::size_t size);
			void Free(char* copy);
			UInt32 ParseNumeric(const char* name);
			hostent* LookupHost(const char* name);
		}
	}
}
