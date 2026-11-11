#include <winsock2.h>
#include "GameWorldCollections.hpp"
#include "entity/World.hpp"
#include "startup/GameServer.hpp"
#include <new>

namespace F3
{
	BYTE GameJournal::IsTaskOpen(DWORD id) //46C350.
	{
		for (std::list<JE::TaskStatus*>::iterator i = statuses.begin(); i != statuses.end(); ++i)
		{
			JE::TaskStatus* status = *i;
			if (status && status->value4 == id)
			{
				BYTE result = 1;
				if (status->valueC != 1) break;
				return result;
			}
		}
		return 0;
	}
	BYTE GameJournal::IsTaskClosed(DWORD id) //46C390.
	{
		for (std::list<JE::TaskStatus*>::iterator i = statuses.begin(); i != statuses.end(); ++i)
		{
			JE::TaskStatus* status = *i;
			if (status && status->value4 == id)
			{
				if (status->valueC == 2) return 1;
				break;
			}
		}
		return 0;
	}
	LONG GameJournal::GetTaskGroup(DWORD id) //46C3D0.
	{
		for (std::list<JE::TaskStatus*>::iterator i = statuses.begin(); i != statuses.end(); ++i)
		{
			JE::TaskStatus* status = *i;
			if (status && status->value4 == id) return LONG(status->value8);
		}
		return -1;
	}
	void GameJournal::OpenTask(DWORD id, DWORD value)
	{
		for (std::list<JE::TaskStatus*>::iterator i = statuses.begin(); i != statuses.end(); ++i)
		{
			JE::TaskStatus* status = *i;
			if (status->value4 == id)
			{
				status->value8 = value;
				status->valueC = 1;
				return;
			}
		}
		JE::TaskStatus* status = new JE::TaskStatus;
		status->value4 = id;
		status->value8 = value;
		status->valueC = 1;
		statuses.push_back(status);
	}
	void GameJournal::CloseTask(DWORD id, DWORD value)
	{
		for (std::list<JE::TaskStatus*>::iterator i = statuses.begin(); i != statuses.end(); ++i)
		{
			JE::TaskStatus* status = *i;
			if (status->value4 == id)
			{
				status->value8 = value;
				status->valueC = 2;
				return;
			}
		}
		JE::TaskStatus* status = new JE::TaskStatus;
		status->value4 = id;
		status->value8 = value;
		status->valueC = 2;
		statuses.push_back(status);
	}
	void GameJournal::AddEntry(DWORD task, DWORD entry)
	{
		for (std::list<JE::JournalEntry*>::iterator i = entries.begin(); i != entries.end(); ++i)
		{
			JE::TaskEntry* item = dynamic_cast<JE::TaskEntry*>(*i);
			if (item && item->value4 == task && item->value8 == entry) return;
		}
		JE::TaskEntry* item = new JE::TaskEntry;
		item->value8 = entry;
		item->value4 = task;
		entries.push_back(item);
	}

	void GameJournal::ClearGameEntries()
	{
		std::list<JE::JournalEntry*>::iterator i = entries.begin();
		while (i != entries.end())
		{
			JE::TaskEntry* task = dynamic_cast<JE::TaskEntry*>(*i);
			if (task)
			{
				std::list<JE::JournalEntry*>::iterator next = entries.erase(i);
				delete task; // TaskEntry removal still precedes its destructor.
				i = next;
			}
			else ++i;
		}
		for (std::list<JE::TaskStatus*>::iterator status = statuses.begin(); status != statuses.end(); ++status)
			if (*status) delete *status;
		statuses.clear();
		ClearRemainingEntries(); //46C541, actual shared owning-list call after statuses.
	}

	void GameServer::OpenTask(DWORD first, DWORD second)
	{
		JE::Journal* current = JE::GetWorldInstance()->GetJournal();
		GameJournal* journal = dynamic_cast<GameJournal*>(current);
		if (!journal) return;
		journal->OpenTask(first, second);
		NotifyTaskOpen(first, second);
	}
	void GameServer::NotifyTaskOpen(DWORD first, DWORD second)
	{
		JE::Net::Packet notification;
		notification.field0x30 = 97;
		notification.WriteDword(first);
		notification.WriteDword(second);
		Broadcast(notification, 1);
	}

	void GameServer::CloseTask(DWORD first, DWORD second)
	{
		JE::Journal* current = JE::GetWorldInstance()->GetJournal();
		GameJournal* journal = dynamic_cast<GameJournal*>(current);
		if (!journal) return;
		journal->CloseTask(first, second);
		NotifyTaskClosed(first, second);
	}
	void GameServer::NotifyTaskClosed(DWORD first, DWORD second)
	{
		JE::Net::Packet notification;
		notification.field0x30 = 98;
		notification.WriteDword(first);
		notification.WriteDword(second);
		Broadcast(notification, 1);
	}

	void GameServer::AddEntry(DWORD first, DWORD second)
	{
		JE::Journal* current = JE::GetWorldInstance()->GetJournal();
		GameJournal* journal = dynamic_cast<GameJournal*>(current);
		if (!journal) return;
		journal->AddEntry(first, second);
		NotifyEntryAdded(first, second);
	}
	void GameServer::NotifyEntryAdded(DWORD first, DWORD second)
	{
		JE::Net::Packet notification;
		notification.field0x30 = 99;
		notification.WriteDword(first);
		notification.WriteDword(second);
		Broadcast(notification, 1);
	}
}
