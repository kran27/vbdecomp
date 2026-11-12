#include "ResourceConfigurationRead.hpp"
#include "ConfigurationParsePrivate.hpp"
#include "resources/ResourceSystem.hpp"

namespace JE
{
	namespace Configuration
	{
		unsigned char __cdecl ReadResourceConfigurationFile(std::string sFilename,
			Settings& settings)
		{
			settings.clear();
			Resources::ResourceFileHandle* const pFile =
				Resources::g_pResourceSystem->OpenStream(sFilename.c_str(), 1);
			if (!pFile)
				return 0;
			if (!Resources::g_pResourceSystem->Seek(pFile, 0, 2))
			{
				Resources::g_pResourceSystem->Release(pFile, 0);
				return 0;
			}
			const Net::UInt32 nLength = Resources::g_pResourceSystem->Tell(pFile);
			if (!Resources::g_pResourceSystem->Seek(pFile, 0, 0))
			{
				Resources::g_pResourceSystem->Release(pFile, 0);
				return 0;
			}
			char* const pBuffer = new char[nLength + 1u];
			if (!Resources::g_pResourceSystem->Read(pBuffer, nLength, pFile))
			{
				Resources::g_pResourceSystem->Release(pFile, 0);
				// Original failed-read behavior leaks the allocated buffer.
				return 0;
			}
			Resources::g_pResourceSystem->Release(pFile, 0);
			pBuffer[nLength] = '\n';
			const unsigned char nResult = Detail::ParseBuffer(pBuffer, settings, nLength);
			delete[] pBuffer;
			return nResult;
		}

	}
}
