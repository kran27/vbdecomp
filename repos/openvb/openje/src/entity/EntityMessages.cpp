#include "Entity.hpp"
#include <cstdlib>
#include <cstring>
#include <new>

namespace JE
{
	void Entity::EnqueueMessage(DWORD from, DWORD message, const cell* arguments, DWORD count)
	{
		EntityMessage* entry = new EntityMessage;
		entry->cellCount = count + 2;
		entry->values = new cell[entry->cellCount];
		entry->values[0] = static_cast<cell>(from);
		entry->values[1] = static_cast<cell>(message);
		memcpy(entry->values + 2, arguments, sizeof(cell) * count);
		messages.push_back(entry); //54EE60 publishes only after the entire copied payload.
	}

	DWORD Entity::MessageArgCount() const
	{
		if (!currentMessage) return 0;
		return currentMessage->cellCount - 2;
	}

	cell Entity::MessageGetArg(LONG index) const
	{
		if (!currentMessage || static_cast<LONG>(currentMessage->cellCount - 2) < index) return 0;
		//549305 is signed JL; -2/-1 expose the header, equality is an original
		//one-past-read domain. Do not turn the observed predicate into a clamp.
		return currentMessage->values[index + 2];
	}

	void Entity::DispatchMessage()
	{
		if (messages.empty()) return;
		std::list<EntityMessage*>::iterator first = messages.begin();
		currentMessage = *first;
		cell result;
		RunScript("OnMessage", &result, 2, currentMessage->values);
		messages.erase(first); //54C67B..6A2 removes the held node after the callback.
		if (currentMessage)
		{
			free(currentMessage->values);
			delete currentMessage;
		}
		currentMessage = NULL; //Reloaded owner after callback, then cleared54C6C4.
	}

	void Entity::DiscardMessage()
	{
		std::list<EntityMessage*>::iterator first = messages.begin();
		if (first == messages.end()) return;
		EntityMessage* entry = *first;
		messages.erase(first);
		if (entry)
		{
			delete[] entry->values;
			delete entry;
		}
	}
}
