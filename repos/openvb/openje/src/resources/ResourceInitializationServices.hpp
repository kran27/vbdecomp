#pragma once

#include "network/Types.hpp"
#include <fstream>

namespace JE
{
	namespace Resources
	{
		class ResourceIndex;
		// Real MSVC stream/heap APIs in production. Comparisons replace these
		// boundaries; no original stream/FILE/runtime layout is reconstructed.
		namespace ResourceInitializationServices
		{
			void Open(std::ifstream& input, const char* pszPath);
			bool IsOpen(const std::ifstream& input);
			void Seek(std::ifstream& input, Net::Int32 nOffset, int nOrigin);
			Net::UInt32 Tell(std::ifstream& input);
			void Read(std::ifstream& input, void* pOutput, Net::UInt32 nBytes);
			void Close(std::ifstream& input);
			void* Allocate(Net::UInt32 nBytes);
			void Free(void* pBuffer);
			void Upsert(ResourceIndex& index, const char* pszName, Net::UInt32 nType,
				Net::UInt32 nFlags, const char* pszPath, Net::UInt32 nOrdinal);
		}
	}
}
