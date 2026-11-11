#pragma once

#include "entity/WorldCollections.hpp"

namespace JE { class Server; }

namespace F3
{
	class GameParty : public JE::Party
	{
	public:
		GameParty(); // 0x46CD00; no additional virtual destructor.
		virtual BYTE Add(JE::Entity* entity); // 0x46CD30; first insertion selects entity.
		virtual BYTE Remove(JE::Entity* entity); // 0x46CD20, forwards base; selection stays unchanged.
		JE::Entity* selectedEntity; // Original+14, initialNULL; dereferenced46C980.
	};
	class GameJournal : public JE::Journal
	{
	public:
		GameJournal(); // 0x46C410.
		virtual ~GameJournal(); // 0x46C550, mislabeled std::locale::_Locimp in database.
		virtual void Cleanup(); // 0x46C5E0.
		void ClearGameEntries(); // 0x46C480; TaskEntry first, statuses, remaining entries.
		void OpenTask(DWORD id, DWORD value); //46C7A0, existing-or-new status1.
		void CloseTask(DWORD id, DWORD value); //46C840, existing-or-new status2.
		void AddEntry(DWORD task, DWORD entry); //46C6C0, unique TaskEntry pair.
		BYTE IsTaskOpen(DWORD id); //46C350, first matching status has state1.
		BYTE IsTaskClosed(DWORD id); //46C390, first matching status has state2.
		LONG GetTaskGroup(DWORD id); //46C3D0, first matching value8 or-1.
		std::list<JE::TaskStatus*> statuses; // Original+10, owning.
	private:
		void ClearRemainingEntries(); //5A1FF0, shared folded owning-list body; native name unknown.
	};
	class GameEncyclopedia : public JE::Encyclopedia
	{
	public:
		GameEncyclopedia(); // Factory46F8E0: base constructor and derived vtable only.
		virtual ~GameEncyclopedia(); // 0x46F100; added derived virtual slot46F0E0.
		virtual void Initialize(); // 0x462950, base initialize then Read("index.enc").
	};
	class GameStoreManager : public JE::StoreManager
	{
	public:
		GameStoreManager(); // Factory46F930: base constructor and derived vtable only.
		virtual ~GameStoreManager(); // 0x46F130; added derived virtual slot46F110.
		virtual void Cleanup(); // 0x46F0D0, forwards actual base Cleanup.
	};
	class GameWorldMap : public JE::WorldMap
	{
	public:
		GameWorldMap(); // Factory46F980: base constructor and derived vtable only.
		virtual ~GameWorldMap(); // 0x46F180.
		virtual void Cleanup(); // 0x4703E0, forwards actual base Cleanup.
	};
}
