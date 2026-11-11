#include "DialogueManager.hpp"
#include <cstring>

namespace JE
{
	JE::Net::UInt8 DialogueManager::WasLastNodeName(const std::string& name) //55C220.
	{
		DialogueState& state = entries.find(field0x4)->second;
		char upper[260];
		std::strcpy(upper, name.c_str());
		::_strupr(upper);
		return state.field0x34.compare(upper) == 0;
	}

	JE::Net::UInt32 DialogueManager::GetPCSpeaker(JE::Net::UInt32 ownerID) const //55BFA0.
	{
		for (DialogueEntries::const_iterator entry = entries.begin(); entry != entries.end(); ++entry)
			if (entry->second.field0xC == ownerID) return entry->second.field0x8;
		return 0;
	}
}
