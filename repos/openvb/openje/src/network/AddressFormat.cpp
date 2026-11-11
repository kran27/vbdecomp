#include "Address.hpp"
// Original resolver calls use the WinSock import-library thunk.
#define WINSOCK_API_LINKAGE
#include <winsock2.h>
#include <cstdio>
#include <cstring>

namespace JE { namespace Net
{
	char* Address::Format(char* output,UInt32 capacity,UInt8 numericOnly) const //5ACAF0.
	{
		if (capacity < 22) throw AddressException("Buffer overflow converting network address to string.");
		if (!numericOnly)
		{
			sockaddr_in native;
			ToSockaddr(native);
			//Original passes all16 bytes, including untouched sin_zero.
			hostent* host = ::gethostbyaddr(reinterpret_cast<const char*>(&native),16,AF_INET);
			if (host && host->h_name)
			{
				UInt32 length = UInt32(std::strlen(host->h_name));
				if (length + 6 < capacity)
				{
					std::strcpy(output,host->h_name);
					std::sprintf(output+length,":%d",port);
					return output;
				}
			}
		}
		std::sprintf(output,"%u.%u.%u.%u:%u",bytes[0],bytes[1],bytes[2],bytes[3],port);
		return output;
	}
} }
