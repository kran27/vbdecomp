#include "Configuration.hpp"
#include "ConfigurationParsePrivate.hpp"

#include <io.h>
#include <new>

namespace JE
{
	namespace Configuration
	{
		// 0x4E0820. See docs/functions/F3-004e0820-ReadConfigurationFile.md.
		unsigned char __cdecl ReadFile(std::string sFilename, Settings& settings)
		{
			settings.clear();
			FILE* const pFile = FileServices::Open(sFilename.c_str(), "rb");
			if (!pFile)
			{
				return 0;
			}
			const int nDescriptor = FileServices::FileNumber(pFile);
			const Net::UInt32 nLength =
				static_cast<Net::UInt32>(FileServices::FileLength(nDescriptor));
			// Native 4E08C8 uses array new for exactly length+1, without extra padding.
			char* const pBuffer = new char[nLength + 1u];
			if (FileServices::Read(pBuffer, nLength, 1, pFile) != 1)
			{
				// Original game behavior leaks both resources on a failed fread.
				return 0;
			}
			FileServices::Close(pFile);
			pBuffer[nLength] = '\n';
			const unsigned char nResult = Detail::ParseBuffer(pBuffer, settings, nLength);
			delete[] pBuffer; //Native 4E0920 uses the actual array-delete entry.
			return nResult;
		}

		namespace FileServices
		{
			FILE* __cdecl Open(const char* pFilename, const char* pMode)
			{
				return std::fopen(pFilename, pMode);
			}
			int __cdecl FileNumber(FILE* pFile)
			{
				return (::_fileno)(pFile); //Call the real API despite VC7 stdio's function-like macro.
			}
			long __cdecl FileLength(int nDescriptor)
			{
				return ::_filelength(nDescriptor);
			}
			void* __cdecl Allocate(Net::UInt32 nBytes)
			{
				// Storage adaptation: the scanner can write a NUL one byte
				// past its logical LF sentinel. Reserve that byte without seeding it.
				// Keep the caller's original DWORD allocation request/scan length.
				const std::size_t nPadding = nBytes == static_cast<Net::UInt32>(-1) ? 0u : 1u;
				return ::operator new(static_cast<std::size_t>(nBytes) + nPadding);
			}
			std::size_t __cdecl Read(void* pBuffer, std::size_t nSize,
				std::size_t nCount, FILE* pFile)
			{
				return std::fread(pBuffer, nSize, nCount, pFile);
			}
			int __cdecl Close(FILE* pFile)
			{
				return std::fclose(pFile);
			}
			void __cdecl Free(void* pBuffer)
			{
				::operator delete(pBuffer);
			}
		}
	}
}
