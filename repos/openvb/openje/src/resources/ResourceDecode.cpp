#include "ResourceDecode.hpp"
#include "../filesystem/Directories.hpp"

#include <new>
#include <stdlib.h>
#include <zlib.h>

namespace JE
{
	namespace Resources
	{
		namespace ResourceDecodeServices
		{
			unsigned char CreateFolder(const char* szPath) { return JE::CreateDirectories(szPath); }
			void* OpenCompressed(const char* szPath, const char* szMode) { return gzopen(szPath, szMode); }
			int ReadCompressed(void* input, void* buffer, unsigned int size)
			{
				return gzread(static_cast<gzFile>(input), buffer, size);
			}
			void CloseCompressed(void* input) { gzclose(static_cast<gzFile>(input)); }
			void OpenOutput(std::ofstream& output, const char* szPath)
			{
				output.open(szPath, std::ios::out | std::ios::binary);
			}
			bool IsOpen(const std::ofstream& output) { return output.is_open(); }
			void Write(std::ofstream& output, const void* buffer, unsigned int size)
			{
				output.write(static_cast<const char*>(buffer), size);
			}
			void CloseOutput(std::ofstream& output) { output.close(); }
			void* Allocate(std::size_t size) { return new unsigned char[size]; }
			void Free(void* buffer) { ::operator delete(buffer); }
		}

		// Original returns AL=0/1; IDA's inferred void return is incorrect.
		unsigned char DecodeCompressedFile(const char* szSource, const char* szDestination,
			std::string& error)
		{
			std::ofstream output;
			char number[32];
			const std::string destination(szDestination);
			const std::size_t separator = destination.find_last_of('\\');
			if (separator != std::string::npos)
			{
				if (!ResourceDecodeServices::CreateFolder(destination.substr(0, separator).c_str()))
				{
					error = "Couldn't Create Folder for file ";
					error += destination;
					return 0;
				}
			}

			void* input = ResourceDecodeServices::OpenCompressed(szSource, "rb");
			if (!input)
			{
				error = "Couldn't open file ";
				error += szSource;
				return 0;
			}
			ResourceDecodeServices::OpenOutput(output, szDestination);
			if (!ResourceDecodeServices::IsOpen(output))
			{
				// The original reports the source even when opening the destination fails.
				error = "Couldn't open file ";
				error += szSource;
				ResourceDecodeServices::CloseCompressed(input);
				return 0;
			}
			void* buffer = ResourceDecodeServices::Allocate(65536);
			if (!buffer)
			{
				error = "Couldn't allocate ";
				error += ::_itoa(65536, number, 10);
				ResourceDecodeServices::CloseOutput(output);
				ResourceDecodeServices::CloseCompressed(input);
				return 0;
			}
			int count;
			do
			{
				count = ResourceDecodeServices::ReadCompressed(input, buffer, 65536);
				if (count == -1)
				{
					error = "Error read file ";
					error += szSource;
					ResourceDecodeServices::Free(buffer);
					ResourceDecodeServices::CloseOutput(output);
					ResourceDecodeServices::CloseCompressed(input);
					return 0;
				}
				// Includes the zero-byte write at EOF; write/close statuses are ignored.
				ResourceDecodeServices::Write(output, buffer, static_cast<unsigned int>(count));
			}
			while (count == 65536);
			ResourceDecodeServices::Free(buffer);
			ResourceDecodeServices::CloseOutput(output);
			ResourceDecodeServices::CloseCompressed(input);
			return 1;
		}
	}
}
