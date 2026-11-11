#include "DialogueManager.hpp"

namespace JE
{
	JE::Net::UInt32 DialogueManager::GetSpeakerID() const //55C1C0.
	{
		DialogueEntries::const_iterator entry = entries.find(field0x4);
		return entry->second.field0xC;
	}

	void DialogueManager::SetSpeakerID(JE::Net::UInt32 entityID) //55C1E0.
	{
		entries.find(field0x4)->second.field0xC = entityID;
	}

	void DialogueManager::SetNextSpeakerID(JE::Net::UInt32 entityID) //55C200.
	{
		entries.find(field0x4)->second.field0x10 = entityID;
	}

	void DialogueManager::RegisterLinkToOtherDialogue(const std::string& script, const std::string& function) //55C4E0.
	{
		DialogueState& state = entries.find(field0x4)->second;
		state.field0x50.field0x24 = script; //55C504 publishes the deferred script name first.
		state.field0x50.field0x8 = function;
	}
}
