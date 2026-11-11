#include "EngineServer.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/Entity.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "scripting/ScriptSmall.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <new>
#include <stdexcept>

namespace JE
{

	void DialogueManager::WriteUpdate(Net::Packet& output) //55E110.
	{
		JE::Net::UInt8 activeCount = 0;
		DialogueEntries::iterator entry = entries.begin();
		for (; entry != entries.end(); ++entry)
		{
			DialogueState state(entry->second); //55E050, ordinary strings/maps; borrowed script.
			if (state.field0x50.field0x0 == 2) ++activeCount;
		}
		output.WriteByte(activeCount);
		entry = entries.begin();
		while (activeCount > 0)
		{
			DialogueState state(entry->second);
			if (state.field0x50.field0x0 != 2)
			{
				++entry;
				continue;
			}
			output.WriteByte(state.field0x0);
			output.WriteByte(state.field0x50.field0x4);
			output.WriteByte(static_cast<JE::Net::UInt8>(GetWorldInstance()->GetParty()->members.size()));
			output.WriteDword(state.field0xC);
			std::vector<Entity*> members(GetWorldInstance()->GetParty()->members); //55C8D0, copied after count/speaker writes.
			for (std::vector<Entity*>::iterator member = members.begin(); member != members.end(); ++member)
			{
				DWORD id = (*member)->entityID;
				output.WriteDword(id);
				std::map<JE::Net::UInt32,JE::Net::UInt32>::iterator text = state.field0x50.field0x40.find(id);
				if (text != state.field0x50.field0x40.end()) output.WriteDword(text->second);
				else output.WriteSignedDword(0);
				std::map<JE::Net::UInt32,DialogueReply> replies;
				std::map<JE::Net::UInt32,std::map<JE::Net::UInt32,DialogueReply> >::iterator found =
					state.field0x50.field0x4C.find(id);
				if (found != state.field0x50.field0x4C.end()) replies = found->second;
				JE::Net::UInt8 count = static_cast<JE::Net::UInt8>(replies.size());
				output.WriteByte(count);
				if (count > 0)
					for (std::map<JE::Net::UInt32,DialogueReply>::iterator reply = replies.begin(); reply != replies.end(); ++reply)
					{
						DialogueReply value(reply->second); //55C390 copies both real strings too.
						output.WriteDword(value.field0x0);
						output.WriteDword(value.field0x4);
					}
			}
			--activeCount;
			//55E576..55E5B0 decrements only the BYTE count. The original advances
			//the manager iterator only for an inactive record, repeating the first
			//active record when more than one active context is counted.
		}
	}

	void DialogueManager::EndDialogue(DialogueEntries::iterator& entry) //55F0A0.
	{
		Net::Packet notification;
		DialogueState& state = entry->second;
		notification.WriteByte(state.field0x0);
		static_cast<Server*>(F3::g_pNetworkInstance->server)->SendDialogueEnd(notification);
		if (state.script) delete state.script;
		entry = entries.erase(entry);
	}

	void DialogueManager::Update() //55F3E0, called after the World server frame by552B60.
	{
		DialogueEntries::iterator entry = entries.begin();
		while (entry != entries.end())
		{
			DialogueState& state = entry->second;
			DialogueContext& context = state.field0x50;
			if (context.field0x58 == 1)
			{
				EndDialogue(entry);
				continue;
			}
			if (context.field0x24 != "")
			{
				std::string filename = context.field0x24 + ".dlg";
				Script_Small* replacement = new Script_Small(state.field0xC);
				DWORD loaded = replacement->Load(filename.c_str(), 1);
				if (!loaded)
				{
					delete replacement;
					FatalError("Jump Link Failed: Unable to load dialogue script %s.\n", filename.c_str());
				}
				if (state.script) delete state.script;
				state.script = replacement;
				context.field0x24 = "";
				context.field0x0 = 4;
				Entity* first = GetEntityPtr(state.field0xC);
				Entity* second = GetEntityPtr(state.field0x10);
				if (!second || second->mapHandle != first->mapHandle)
				{
					Net::Packet notification;
					notification.WriteByte(state.field0x0);
					static_cast<Server*>(F3::g_pNetworkInstance->server)->SendDialogueEnd(notification);
					if (state.script)
					{
						delete state.script;
						state.script = NULL;
					}
					entry = entries.erase(entry);
					continue;
				}
				SetSpeakerID(state.field0x10);
				state.field0x10 = 0;
			}
			Script_Small* script = state.script;
			if (script)
			{
			field0x4 = state.field0x0;
			context.field0x4C.clear();
			cell result;
			if (context.field0x0 == 0)
			{
				script->Run(context.field0x8.c_str(), &result);
				context.field0x0 = 1;
			}
			else if (context.field0x0 == 1 || context.field0x0 == 2)
			{
				Party* party = GetWorldInstance()->GetParty();
				for (std::vector<Entity*>::iterator member = party->members.begin(); member != party->members.end(); ++member)
					script->Run(context.field0x8.c_str(), &result);
				context.field0x0 = 2;
			}
			else if (context.field0x0 == 3)
			{
				if (context.field0x8 != "") script->Run(context.field0x8.c_str(), &result);
				context.field0x0 = 1;
			}
			else if (context.field0x0 == 4)
			{
				script->Run(context.field0x8.c_str(), &result);
				context.field0x0 = 1;
			}
			if (context.field0x8 == "EndDialogue") EndDialogue(entry);
			else ++entry;
			}
		}
		field0x4 = 0;
	}
}
