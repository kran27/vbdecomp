#include "Address.hpp"

// Original calls use the WinSock import-library thunks.
#define WINSOCK_API_LINKAGE
#include <winsock2.h>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <new>

namespace JE
{
	namespace Net
	{
		Address& Address::operator=(const Address& source)
		{
			bytes[0] = source.bytes[0];
			bytes[1] = source.bytes[1];
			bytes[2] = source.bytes[2];
			bytes[3] = source.bytes[3];
			port = source.port;
			return *this;
		}

		UInt8 Address::ToSockaddr(sockaddr_in& output) const
		{
			output.sin_family = AF_INET;
			output.sin_port = htons(port);
			UInt8* octets = reinterpret_cast<UInt8*>(&output.sin_addr.s_addr);
			octets[0] = bytes[0];
			octets[1] = bytes[1];
			UInt8 third = bytes[2];
			octets[2] = third;
			octets[3] = bytes[3];
			return third;
		}

		unsigned char Address::Assign(const sockaddr_in& address)
		{
			port = ntohs(address.sin_port);
			const unsigned char* source = reinterpret_cast<const unsigned char*>(&address.sin_addr.s_addr);
			bytes[0] = source[0];
			bytes[1] = source[1];
			bytes[2] = source[2];
			bytes[3] = source[3];
			return bytes[2];
		}

		int Address::Compare(const Address& address, UInt8 includePort) const
		{
			const bool same = address.bytes[0] == bytes[0] && address.bytes[1] == bytes[1]
				&& address.bytes[2] == bytes[2] && address.bytes[3] == bytes[3];
			if (includePort) return same && address.port == port;
			return same;
		}

		void Address::Resolve(const char* name)
		{
			bytes[3] = 0;
			bytes[2] = 0;
			bytes[1] = 0;
			bytes[0] = 0;
			port = 0;
			char* copy = AddressServices::Allocate(std::strlen(name) + 1);
			std::strcpy(copy, name);
			sockaddr_in native;
			std::memset(&native, 0, sizeof(native));
			native.sin_family = AF_INET;
			char* colon = std::strchr(copy, ':');
			if (colon)
			{
				native.sin_port = htons(static_cast<u_short>(std::atol(colon + 1)));
				*colon = 0;
			}
			if (!::_stricmp(copy, "localhost"))
			{
				unsigned char* octets = reinterpret_cast<unsigned char*>(&native.sin_addr.s_addr);
				octets[0] = 127; octets[1] = 0; octets[2] = 0; octets[3] = 1;
			}
			else if (std::isdigit(*copy))
				native.sin_addr.s_addr = AddressServices::ParseNumeric(copy);
			else
			{
				hostent* host = AddressServices::LookupHost(copy);
				if (!host)
				{
					AddressServices::Free(copy);
					throw AddressException("Unable to retrieve host information.");
				}
				std::memcpy(&native.sin_addr.s_addr, host->h_addr_list[0], sizeof(native.sin_addr.s_addr));
			}
			Assign(native);
			AddressServices::Free(copy);
		}

		namespace AddressServices
		{
			inline char* Allocate(std::size_t size) { return static_cast<char*>(::operator new(size)); }
			inline void Free(char* copy) { ::operator delete(copy); }
			inline UInt32 ParseNumeric(const char* name) { return inet_addr(name); }
			inline hostent* LookupHost(const char* name) { return gethostbyname(name); }
		}
	}
}
