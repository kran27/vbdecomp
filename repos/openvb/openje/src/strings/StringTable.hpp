#pragma once

#include "network/Types.hpp"
#include <cstddef>
#include <fstream>
#include <string>

namespace JE
{
	namespace SS
	{
		struct StfEntry
		{
			JE::Net::UInt32 offset;
			JE::Net::UInt32 length;
			JE::Net::UInt32 unknown8;
			JE::Net::UInt32 unknown12;
		};
		typedef char StfEntryIsFourDwords[(sizeof(StfEntry) == 16) ? 1 : -1];

		StfEntry ReadStfEntry(const unsigned char* entries, JE::Net::UInt32 index);

		namespace StringTableServices
		{
			void Open(std::ifstream& input, const char* path);
			bool IsOpen(const std::ifstream& input);
			void Read(std::ifstream& input, void* output, JE::Net::UInt32 bytes);
			void Seek(std::ifstream& input, JE::Net::Int32 offset);
			void Close(std::ifstream& input);
			void* Allocate(JE::Net::UInt32 bytes);
			void Free(void* pointer);
		}
	}
}
