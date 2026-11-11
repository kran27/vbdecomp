#include "DialogueManager.hpp"
#include "scripting/ScriptSmall.hpp"

#include <new>
#include <stdexcept>

namespace JE
{
	DialogueManager::DialogueManager()
	{
		field0x4 = 0;
		field0x5 = 1;
	}

	DialogueManager::~DialogueManager()
	{
		while (!entries.empty())
		{
			DialogueEntries::iterator captured = entries.begin();
			DialogueScript* script = captured->second.script;
			if (script)
			{
				DialogueManagerServices::DeleteScript(script);
				captured->second.script = NULL;
			}
			DialogueManagerServices::Erase(entries,captured);
		}
		// Actual map/member destruction owns the remaining library lifetime.
	}

	void DialogueManager::AdvanceDialogueID() //55BAB0.
	{
		if (++field0x5 == 0) field0x5 = 1;
	}

	void DialogueManager::Initialize() { }

	namespace DialogueManagerServices
	{
		void DeleteScript(DialogueScript* script)
		{
			delete script;
		}
		void Erase(DialogueEntries& entries, DialogueEntries::iterator entry) { entries.erase(entry); }
	}

	namespace DialogueFactoryServices
	{
		void* Allocate(std::size_t size) { return ::operator new(size); }
		void Construct(DialogueManager* manager) { ::new (manager) DialogueManager; }
		void Free(void* block) throw() { ::operator delete(block); }
	}
}
