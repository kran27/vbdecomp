#pragma once

#include <cstddef>
#include <fstream>
#include <string>

namespace JE
{
	namespace Resources
	{
		// 0x537EA0: proprietary file/error policy around the real zlib gzip API.
		unsigned char DecodeCompressedFile(const char* szSource, const char* szDestination,
			std::string& error);

		namespace ResourceDecodeServices
		{
			unsigned char CreateFolder(const char* szPath);
			void* OpenCompressed(const char* szPath, const char* szMode);
			int ReadCompressed(void* input, void* buffer, unsigned int size);
			void CloseCompressed(void* input);
			void OpenOutput(std::ofstream& output, const char* szPath);
			bool IsOpen(const std::ofstream& output);
			void Write(std::ofstream& output, const void* buffer, unsigned int size);
			void CloseOutput(std::ofstream& output);
			void* Allocate(std::size_t size);
			void Free(void* buffer);
		}
	}
}
