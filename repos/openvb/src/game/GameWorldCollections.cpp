#include "GameWorldCollections.hpp"

namespace JE
{
	TaskEntry::TaskEntry() {}
	TaskEntry::~TaskEntry() {}
}

namespace F3
{
	GameParty::GameParty() : selectedEntity(NULL) {}
	BYTE GameParty::Add(JE::Entity* entity)
	{
		BYTE result = JE::Party::Add(entity);
		if (result == 1 && members.size() == 1) selectedEntity = entity;
		return result;
	}
	BYTE GameParty::Remove(JE::Entity* entity) { return JE::Party::Remove(entity); }
	GameJournal::GameJournal() { statuses.clear(); }
	GameJournal::~GameJournal()
	{
		ClearGameEntries();
	}
	void GameJournal::ClearRemainingEntries()
	{
		for (std::list<JE::JournalEntry*>::iterator i = entries.begin(); i != entries.end(); ++i)
			if (*i) delete *i;
		entries.clear();
	}
	void GameJournal::Cleanup()
	{
		ClearGameEntries();
		JE::Journal::Cleanup();
	}
	void GameEncyclopedia::Initialize()
	{
		JE::Encyclopedia::Initialize();
		Read("index.enc");
	}
	void GameStoreManager::Cleanup() { JE::StoreManager::Cleanup(); }
	void GameWorldMap::Cleanup() { JE::WorldMap::Cleanup(); }
}
