#include "DialogueManager.hpp"
#include <cstring>
#include <utility>

namespace JE
{
	void DialogueManager::RegisterNodeText(JE::Net::UInt32 textID) //55C890.
	{
		DialogueState& state=entries.find(field0x4)->second;
		std::map<Net::UInt32,Net::UInt32>::value_type value(state.field0x8,textID);
		state.field0x50.field0x40.insert(value);
	}
	void DialogueManager::SetNodeTextFunction(const std::string& name) //55D910.
	{
		DialogueState& state=entries.find(field0x4)->second;
		state.field0x50.field0x8=name;
		++state.field0x50.field0x4; //55D93A increments the raw BYTE, including wrap to0.
		state.field0x50.field0x40.clear();
		state.field0x50.field0x4C.clear();
	}
	void DialogueManager::RegisterReply(JE::Net::UInt32 id,JE::Net::UInt32 textID,
		const std::string& action,const std::string& nextNode) //55D9A0.
	{
		DialogueState& state=entries.find(field0x4)->second;
		DialogueReply reply;
		reply.field0x0=id;
		reply.field0x4=textID;
		reply.field0x8=action;
		reply.field0x24=nextNode;
		typedef std::map<Net::UInt32,DialogueReply> Replies;
		typedef std::map<Net::UInt32,Replies> Participants;
		Participants::iterator participant = state.field0x50.field0x4C.find(state.field0x8);
		if (participant == state.field0x50.field0x4C.end())
		{
			std::pair<Participants::iterator,bool> inserted = state.field0x50.field0x4C.insert(
				Participants::value_type(state.field0x8, Replies()));
			participant = inserted.first;
		}
		Replies& replies = participant->second;
		replies.insert(Replies::value_type(reply.field0x0,reply));
	}
	void DialogueManager::SetCurrentNodeName(const std::string& name) //55C420.
	{
		DialogueState& state=entries.find(field0x4)->second;
		state.field0x34=state.field0x18; //Previous name is published before copying/normalizing the next.
		char buffer[260];
		std::strcpy(buffer,name.c_str());
		::_strupr(buffer); //62B180 is the actual CRT uppercase API; reuse the supplied VC7 CRT.
		state.field0x18=buffer;
	}
}
