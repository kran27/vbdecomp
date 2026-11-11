// OpenJE - proprietary configuration behavior with the actual MSVC libraries.
#pragma once

#include "ConfigurationTypes.hpp"

#include <cstddef>
#include "network/Types.hpp"
#include <cstdio>
#include <cstring>
#include <map>
#include <string>

namespace JE
{
	namespace Configuration
	{
		// Public compatibility API for 0x4E0540 behavior. The production body is private.
		// Mutates input, including sentinel/next-line bytes. Keys require
		// a preceding section. The original unchecked scans are not a safe-input API.
		unsigned char __cdecl ParseBuffer(char* pBuffer, Settings& settings,
			Net::UInt32 nLength);

		// 0x4E0820. Clears settings before fopen. Preserves failed-read leaks and
		// close-before-parse ordering.
		unsigned char __cdecl ReadFile(std::string sFilename, Settings& settings);

		// 0x4DF1C0. Text output in map order. Exact fputs EOF fails; fclose is ignored.
		unsigned char __cdecl WriteFile(std::string sFilename, const Settings& settings);


		// Production Allocate reserves an extra unwritten byte for the parser's
		// end+1 store; the original logical allocation request remains unchanged.
		namespace FileServices
		{
			FILE* __cdecl Open(const char*, const char*);
			int __cdecl FileNumber(FILE*);
			long __cdecl FileLength(int);
			void* __cdecl Allocate(Net::UInt32);
			std::size_t __cdecl Read(void*, std::size_t, std::size_t, FILE*);
			int __cdecl Close(FILE*);
			void __cdecl Free(void*);
		}

		namespace WriteFileServices
		{
			FILE* __cdecl Open(const char*, const char*);
			int __cdecl PutString(const char*, FILE*);
			int __cdecl Close(FILE*);
		}
	}
}
