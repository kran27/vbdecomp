#include "ConfigurationParsePrivate.hpp"

namespace JE
{
	namespace Configuration
	{
		// Public compatibility entry; the native parser is the private Detail body.
		unsigned char __cdecl ParseBuffer(char* pBuffer, Settings& settings,
			Net::UInt32 nLength)
		{
			return Detail::ParseBuffer(pBuffer,settings,nLength);
		}
	}
}
