#include "DialogueManager.hpp"

namespace JE
{
	void DialogueManager::SelectReply(JE::Net::UInt8 id, JE::Net::UInt8 revision,
		JE::Net::UInt32 replyID, JE::Net::UInt32 participantID) //55F850.
	{
		Update(); //55F882: evaluate current replies before checking the incoming revision.
		DialogueEntries::iterator entry = entries.find(id);
		if (entry == entries.end() || entry->second.field0x50.field0x4 != revision) return;
		DialogueState& state = entry->second;
		std::map<JE::Net::UInt32,std::map<JE::Net::UInt32,DialogueReply> >::iterator participant =
			state.field0x50.field0x4C.find(participantID);
		if (participant == state.field0x50.field0x4C.end()) return;
		std::map<JE::Net::UInt32,DialogueReply>::iterator found = participant->second.find(replyID);
		if (found == participant->second.end()) return;
		DialogueReply reply(found->second); //55C390 copies both strings before script callbacks.
		if (!reply.field0x4) reply.field0x4 = 184; //Original local-only default; not written back.
		std::string action(reply.field0x8);
		std::string next(reply.field0x24);
		state.field0x50.field0x0 = 3;
		state.field0x50.field0x8 = action;
		Update();
		//The original retains its entry pointer across the callback/pump. An action
		//that removes this context leaves it dangling; do not turn that into success.
		state.field0x50.field0x8 = next;
		state.field0x8 = participantID; //55F990, after action update and next callback publication.
	}

	void DialogueManager::RequestEndDialogue(JE::Net::UInt8 id) //55C2D0.
	{
		DialogueEntries::iterator entry = entries.find(id);
		if (entry != entries.end()) entry->second.field0x50.field0x58 = 1;
	}
}
