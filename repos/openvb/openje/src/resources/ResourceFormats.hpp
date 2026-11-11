#pragma once

#include "network/Types.hpp"
#include <istream>
#include <string>

namespace JE
{
	namespace Resources
	{
		// Original 0x499F10 removes the final extension, including unknown ones.
		Net::UInt32 ExtractFileType(std::string& sFilename);

		// Actual GRP file fields, not an in-memory engine or standard-library ABI.
		struct GroupEntryRange
		{
			Net::UInt32 offset;
			Net::UInt32 length;
		};
		typedef char GroupEntryRangeWidth[(sizeof(GroupEntryRange) == 8) ? 1 : -1];

		// Original 0x499160 reads a header at the incoming stream position.
		unsigned char ReadGroupEntryRange(std::istream& input,
			const std::string& sGroupFilename, const std::string& sResourceFilename,
			const Net::UInt32& nIndex, GroupEntryRange& range);

		namespace ResourceFormatServices
		{
			void Read(std::istream& input, void* pOutput, Net::UInt32 nBytes);
			void Seek(std::istream& input, Net::Int32 nOffset);
		}
	}
}
