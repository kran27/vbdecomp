#include "StringSystem.hpp"

#include <cstring>

namespace JE
{
	namespace SS
	{
		unsigned char StringSystem::GetByIndex(JE::Net::UInt32 index, char* output)
		{
			const JE::Net::UInt32 count = selectedCount;
			if (index >= count)
			{
				const JE::Net::UInt32 end = count + static_cast<JE::Net::UInt32>(extraStrings.size());
				if (index >= end) return 0;
				std::strcpy(output, extraStrings[index - count].c_str());
				return 1;
			}
			const StfEntry* entry = reinterpret_cast<const StfEntry*>(selectedEntries) + index;
			if (entry->length > 0)
			{
				selectedInput->seekg(static_cast<std::streamoff>(entry->offset), std::ios::beg);
				selectedInput->read(output, reinterpret_cast<const StfEntry*>(selectedEntries)[index].length);
				output[reinterpret_cast<const StfEntry*>(selectedEntries)[index].length] = 0;
				return 1;
			}
			if (selectedEntries != maleEntries && index < maleCount)
			{
				entry = reinterpret_cast<const StfEntry*>(maleEntries) + index;
				if (entry->length > 0)
				{
					maleInput.seekg(static_cast<std::streamoff>(entry->offset), std::ios::beg);
					maleInput.read(output, reinterpret_cast<const StfEntry*>(maleEntries)[index].length);
					output[reinterpret_cast<const StfEntry*>(maleEntries)[index].length] = 0;
					return 1;
				}
			}
			return 0;
		}

		void StringSystem::GetString(JE::Net::UInt32 strref, std::string& output)
		{
			if (!strref) return;
			char buffer[8192];
			if (!GetByIndex(strref - 1, buffer))
				output = "";
			else
				output = buffer;
		}

	}
}
