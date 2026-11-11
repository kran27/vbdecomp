#include "gui/DialogueInterface.hpp"
#include "gui/DialogueWindow.hpp"
#include "network/Packet.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <stdexcept>

namespace JE
{
	BYTE DialogueInterface::AllWindowsHidden() //592630.
	{
		for (std::map<BYTE,DialogueWindow*>::iterator i = windows.begin(); i != windows.end(); ++i)
			if (!i->second->GetHidden()) return 0;
		return 1;
	}

	DialogueInterface::DialogueInterface(const std::string& name) : EngineInterface(name)
	{
		LogDebugString("DIALOGUEINTERFACE INSTANTIATED\n - This debug line is to double check that it's not being created more than once.");
	}
	DialogueInterface::~DialogueInterface() //593530.
	{
		while (!windows.empty())
		{
			std::map<BYTE, DialogueWindow*>::iterator first = windows.begin();
			DialogueWindow* window = first->second;
			windows.erase(first); //Remove ownership before the virtual destructor.
			if (window) delete window;
		}
	}
	DialogueWindow* DialogueInterface::CreateDialogueWindow(BYTE id, BYTE revision) //596780.
	{
		return new DialogueWindow(id, revision, this);
	}
	void DialogueInterface::RemoveDialogue(BYTE id) //593090.
	{
		std::map<BYTE, DialogueWindow*>::iterator entry;
		if ((entry = windows.find(id)) != windows.end())
		{
			DialogueWindow* window = entry->second;
			windows.erase(entry);
			if (window) delete window;
		}
	}
	void DialogueInterface::BeforeUpdate() //5925B0.
	{
		for (std::map<BYTE, DialogueWindow*>::iterator i = windows.begin(); i != windows.end(); ++i)
		{
			DialogueWindow* window = i->second;
			if (!window->GetHidden()) window->UpdatePortrait();
		}
	}
	void DialogueInterface::BeforeDraw() //5925F0.
	{
		for (std::map<BYTE, DialogueWindow*>::iterator i = windows.begin(); i != windows.end(); ++i)
		{
			DialogueWindow* window = i->second;
			if (!window->GetHidden()) window->QueuePortrait();
		}
	}
	void DialogueInterface::ReadUpdate(Net::Packet& packet) //596050.
	{
		Net::UInt8 count;
		packet.ReadByte(count);
		for (; count > 0; --count)
		{
			Net::UInt8 id, revision, members;
			Net::UInt32 speakerID;
			packet.ReadByte(id); packet.ReadByte(revision); packet.ReadByte(members);
			packet.ReadDword(speakerID);
			DialogueWindow* window;
			std::map<BYTE, DialogueWindow*>::iterator entry = windows.find(id);
			if (entry == windows.end())
			{
				window = CreateDialogueWindow(id, revision);
				windows.insert(std::make_pair(id, window));
				AddWindow(window); //Original publishes the actual GUI owner before member reads.
			}
			else window = entry->second;
			window->ClearMembers();
			window->SetDialogueFlag(revision);
			LONG replyCount = 0;
			for (; members > 0; --members)
			{
				Net::UInt32 entityID, textID;
				packet.ReadDword(entityID); packet.ReadDword(textID);
				window->SetMember(entityID, textID);
				Net::UInt8 replies;
				packet.ReadByte(replies);
				replyCount = replies;
				for (; replies > 0; --replies)
				{
					Net::UInt32 replyID, replyText;
					packet.ReadDword(replyID); packet.ReadDword(replyText);
					window->AddReply(entityID, replyID, replyText);
				}
			}
			IWorld* world = g_World;
			Party* party = world->GetParty(); //5962FC: required virtual IWorld+68.
			window->SetParticipants(party->members.front()->entityID, speakerID);
			window->SetHidden(replyCount <= 0); //The final member's original count, including zero members.
		}
	}
}
