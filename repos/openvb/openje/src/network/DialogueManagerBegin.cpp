#include <winsock2.h>
#include "DialogueManager.hpp"
#include "scripting/ScriptSmall.hpp"
#include <new>
#include <utility>

namespace JE
{
	JE::Net::UInt8 DialogueManager::BeginDialogue(const std::string& name,
		JE::Net::UInt32 speakerID, JE::Net::UInt32 listenerID) //55EDB0.
	{
		std::string filename = name + ".dlg";
		Script_Small* script = new Script_Small(speakerID);
		const DWORD loaded = script->Load(filename.c_str(), 1);
		if (loaded == 0)
		{
			std::string message = "DialogueManager::BeginDialogue() - Error loading script ";
			message += filename + "\n";
			OutputDebugStringA(message.c_str());
			delete script;
			return 0;
		}

		DialogueState state; //55DF60 constructs only the actual strings/maps.
		state.field0x0 = field0x5;
		state.script = script;
		state.field0x8 = listenerID;
		state.field0xC = speakerID;
		state.field0x10 = 0;
		state.field0x14 = listenerID;
		state.field0x18 = "";
		state.field0x34 = "";
		state.field0x50.field0x0 = 0;
		state.field0x50.field0x4 = 0;
		state.field0x50.field0x8 = "OnStartDialogue";
		state.field0x50.field0x58 = 0;
		// The original insert does not replace an existing key or reclaim its
		// newly loaded script when the counter collides with a live dialogue.
		//55F032 advances before the inserted pair temporary is destroyed.
		entries.insert(DialogueEntries::value_type(field0x5, state)), AdvanceDialogueID();
		return 1;
	}

	JE::Net::UInt8 DialogueManager::IsInDialogue(JE::Net::UInt32 entityID) const //55BF60.
	{
		for (DialogueEntries::const_iterator entry = entries.begin(); entry != entries.end(); ++entry)
		{
			if (entry->second.field0xC == entityID || entry->second.field0x8 == entityID) return 1;
		}
		return 0;
	}
}
