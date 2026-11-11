#include "StringSystem.hpp"

#include <cstring>
#include <new>

namespace JE
{
	namespace SS
	{
		StfEntry ReadStfEntry(const unsigned char* entries, JE::Net::UInt32 index)
		{
			StfEntry entry;
			std::memcpy(&entry, entries + (index << 4), sizeof(entry));
			return entry;
		}

		unsigned char StringSystem::LoadTable(const std::string& path, std::ifstream& input,
			unsigned char*& entries, JE::Net::UInt32& count)
		{
			StringTableServices::Open(input, path.c_str());
			if (!StringTableServices::IsOpen(input)) return 0;
			JE::Net::UInt32 header[3];
			StringTableServices::Read(input, header, sizeof(header));
			if (header[0] != 3 || header[1] != 1) return 0;
			count = header[2];
			entries = static_cast<unsigned char*>(StringTableServices::Allocate(count << 4));
			// Reload the count after allocation. Neither allocation/read result is checked.
			StringTableServices::Read(input, entries, count << 4);
			return 1;
		}

		namespace StringTableServices
		{
			void Open(std::ifstream& input, const char* path) { input.open(path, std::ios::in | std::ios::binary); }
			bool IsOpen(const std::ifstream& input) { return input.is_open(); }
			void Read(std::ifstream& input, void* output, JE::Net::UInt32 bytes)
			{
				input.read(static_cast<char*>(output), static_cast<std::streamsize>(bytes));
			}
			void Seek(std::ifstream& input, JE::Net::Int32 offset) { input.seekg(offset, std::ios::beg); }
			void Close(std::ifstream& input) { input.close(); }
			void* Allocate(JE::Net::UInt32 bytes) { return ::operator new[](bytes); }
			void Free(void* pointer) { ::operator delete(pointer); }
		}
	}
}
