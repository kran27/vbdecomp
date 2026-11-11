#include "ResourceFormats.hpp"
#include "ResourceSystem.hpp"
#include "diagnostics/DebugOutput.hpp"

#include <cstring>

namespace JE
{
	namespace Resources
	{
		Net::UInt32 ExtractFileType(std::string& sFilename)
		{
			const Net::UInt32 nLength = static_cast<Net::UInt32>(sFilename.size());
			const Net::UInt32 nDot = static_cast<Net::UInt32>(sFilename.rfind('.', nLength));
			// The original 32-bit npos + 1 wraps to zero: a no-dot filename is
			// compared in full, and its stem remains the entire original filename.
			const std::string sExtension = sFilename.substr(nDot + 1U, nLength);
			sFilename.assign(sFilename.substr(0, nDot));

			if (::_stricmp(sExtension.c_str(), "g3d") == 0 || ::_stricmp(sExtension.c_str(), "b3d") == 0) return 100;
			if (::_stricmp(sExtension.c_str(), "dds") == 0 || ::_stricmp(sExtension.c_str(), "bmp") == 0 || ::_stricmp(sExtension.c_str(), "jpg") == 0 || ::_stricmp(sExtension.c_str(), "gif") == 0 || ::_stricmp(sExtension.c_str(), "png") == 0 || ::_stricmp(sExtension.c_str(), "tga") == 0 || ::_stricmp(sExtension.c_str(), "anm") == 0) return 200;
			if (::_stricmp(sExtension.c_str(), "dlg") == 0) return 300;
			if (::_stricmp(sExtension.c_str(), "8") == 0) return 400;
			if (::_stricmp(sExtension.c_str(), "skn") == 0) return 500;
			if (::_stricmp(sExtension.c_str(), "skl") == 0) return 600;
			if (::_stricmp(sExtension.c_str(), "gr2") == 0) return 700;
			if (::_stricmp(sExtension.c_str(), "ini") == 0) return 800;
			if (::_stricmp(sExtension.c_str(), "rlz") == 0) return 900;
			if (::_stricmp(sExtension.c_str(), "rtd") == 0) return 1000;
			if (::_stricmp(sExtension.c_str(), "wav") == 0) return 1100;
			if (::_stricmp(sExtension.c_str(), "amx") == 0) return 1200;
			if (::_stricmp(sExtension.c_str(), "map") == 0) return 1300;
			if (::_stricmp(sExtension.c_str(), "crt") == 0) return 1400;
			if (::_stricmp(sExtension.c_str(), "veg") == 0) return 1500;
			if (::_stricmp(sExtension.c_str(), "int") == 0) return 1600;
			if (::_stricmp(sExtension.c_str(), "rle") == 0) return 1700;
			if (::_stricmp(sExtension.c_str(), "itm") == 0) return 1800;
			if (::_stricmp(sExtension.c_str(), "psf") == 0) return 1900;
			if (::_stricmp(sExtension.c_str(), "gsf") == 0) return 2000;
			if (::_stricmp(sExtension.c_str(), "gls") == 0) return 2100;
			if (::_stricmp(sExtension.c_str(), "sco") == 0) return 2200;
			if (::_stricmp(sExtension.c_str(), "pce") == 0) return 2300;
			if (::_stricmp(sExtension.c_str(), "wea") == 0) return 2400;
			if (::_stricmp(sExtension.c_str(), "arm") == 0) return 2500;
			if (::_stricmp(sExtension.c_str(), "dor") == 0) return 2600;
			if (::_stricmp(sExtension.c_str(), "use") == 0) return 2700;
			if (::_stricmp(sExtension.c_str(), "sst") == 0) return 2800;
			if (::_stricmp(sExtension.c_str(), "str") == 0) return 2900;
			if (::_stricmp(sExtension.c_str(), "enc") == 0) return 3000;
			if (::_stricmp(sExtension.c_str(), "wmp") == 0) return 3100;
			if (::_stricmp(sExtension.c_str(), "pro") == 0) return 3200;
			if (::_stricmp(sExtension.c_str(), "spl") == 0) return 3300;
			if (::_stricmp(sExtension.c_str(), "fnt") == 0) return 3400;
			if (::_stricmp(sExtension.c_str(), "amo") == 0) return 3500;
			if (::_stricmp(sExtension.c_str(), "con") == 0) return 3600;
			if (::_stricmp(sExtension.c_str(), "tok") == 0) return 3700;
			return 0;
		}

		unsigned char ResourceSystem::ReadGroupEntryRange(ResourceDescriptor* descriptor,
			ResourceFileHandle* handle, GroupEntryRange* range)
		{
			Net::UInt32 header[3];
			handle->file->input.read(reinterpret_cast<char*>(header), sizeof(header));
			if (header[0] == 2 && header[1] == 1)
			{
				const Net::UInt32 index = descriptor->ordinal;
				if (index >= header[2])
				{
					JE::FatalError("Groupfile %s entries %d - tried to get %d\n",
						descriptor->name.c_str(), static_cast<Net::Int32>(header[2]),
						static_cast<Net::Int32>(index));
					return 0;
				}
				handle->file->input.seekg(static_cast<std::streamoff>(12U + index * 8U),
					std::ios::beg);
				handle->file->input.read(reinterpret_cast<char*>(range), sizeof(*range));
				return 1;
			}
			else
				JE::FatalError("Groupfile %s incorrect version\n", handle->file->name.c_str());
			return 0;
		}

		unsigned char ReadGroupEntryRange(std::istream& input,
			const std::string& sGroupFilename, const std::string& sResourceFilename,
			const Net::UInt32& nIndex, GroupEntryRange& range)
		{
			// Do not initialize unwritten header bytes or make an unsupported
			// successful short-read interpretation. That original domain is unknown.
			Net::UInt32 header[3];
			ResourceFormatServices::Read(input, header, sizeof(header));
			if (header[0] != 2 || header[1] != 1)
				JE::FatalError("Groupfile %s incorrect version\n", sGroupFilename.c_str());
			if (nIndex >= header[2])
				JE::FatalError("Groupfile %s entries %d - tried to get %d\n",
					sResourceFilename.c_str(), static_cast<Net::Int32>(header[2]),
					static_cast<Net::Int32>(nIndex));

			const Net::UInt32 nPosition = 12U + nIndex * 8U;
			ResourceFormatServices::Seek(input, static_cast<Net::Int32>(nPosition));
			ResourceFormatServices::Read(input, &range, sizeof(range));
			// Neither stream status nor read count is inspected by the original.
			return 1;
		}

		namespace ResourceFormatServices
		{
			void Read(std::istream& input, void* pOutput, Net::UInt32 nBytes)
			{
				input.read(static_cast<char*>(pOutput), static_cast<std::streamsize>(nBytes));
			}

			void Seek(std::istream& input, Net::Int32 nOffset)
			{
				input.seekg(static_cast<std::streamoff>(nOffset), std::ios::beg);
			}
		}
	}
}
