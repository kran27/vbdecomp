#include "Inventory.hpp"
#include <algorithm>
#include <stdexcept>

namespace JE
{
	void Inventory::PackState(Net::Packet&) {} //Both original tables emit no inventory payload.
	void Inventory::UnpackState(Net::Packet&) {} //Original54E944 reaches the independent inbound slot24.

	BYTE Inventory::IsEquipped(Entity*) //591690, original AL0/RET4.
	{
		return 0;
	}

	BYTE Inventory::ContainsID(DWORD id) //55FD60; zero is compared normally, no special invalid-ID gate.
	{
		BYTE result = 0;
		for (std::list<Entity*>::iterator i = items.begin(); i != items.end(); ++i)
			if ((*i)->entityID == id)
			{
				result = 1;
				break;
			}
		return result;
	}

	DWORD Inventory::CountResource(const std::string& name) //55FE20.
	{
		DWORD count = 0;
		for (std::list<Entity*>::iterator i = items.begin(); i != items.end(); ++i)
		{
			Entity* entity = *i;
			if (!::_stricmp(entity->resourceName.c_str(), name.c_str())) ++count;
		}
		return count;
	}

	Entity* Inventory::FindResource(const std::string& name) //55FDA0.
	{
		Entity* result = NULL;
		for (std::list<Entity*>::iterator i = items.begin(); i != items.end(); ++i)
		{
			Entity* entity = *i;
			if (!::_stricmp(entity->resourceName.c_str(), name.c_str()))
			{
				result = entity;
				break;
			}
		}
		return result;
	}
	Inventory::Inventory()
	{
		ownerEntityID = 0;
	}

	BYTE Inventory::GetEquippedSlots(Entity*, std::list<DWORD>&) //436230.
	{
		return 0;
	}

	void Inventory::Add(Entity* entity)
	{
		if (entity->inventoryOwnerID == ownerEntityID) return;
		if (entity->inventoryOwnerID)
			GetEntityPtr(entity->inventoryOwnerID)->inventory->Remove(entity);
		items.push_back(entity);
		OnAdd(entity);
		entity->RemoveFromOctree();
	}

	void Inventory::Remove(Entity* entity)
	{
		std::list<Entity*>::iterator i = std::find(items.begin(), items.end(), entity);
		if (i != items.end())
		{
			Entity* captured = *i;
			items.erase(i);
			OnRemove(captured);
		}
	}

	BYTE Inventory::Contains(Entity* entity)
	{
		BYTE result = 0;
		for (std::list<Entity*>::const_iterator i = items.begin(); i != items.end(); ++i)
			if (*i == entity)
			{
				result = 1;
				break;
			}
		return result;
	}

	void Inventory::OnAdd(Entity* entity)
	{
		entity->SetInventoryOwner(ownerEntityID);
		Entity* owner = GetEntityPtr(ownerEntityID);
		if (owner) entity->SetMap(owner->mapHandle);
	}

	void Inventory::OnRemove(Entity* entity)
	{
		entity->SetInventoryOwner(0);
		entity->SetMap(0);
	}

	Inventory::~Inventory()
	{
		std::list<Entity*>::iterator i = items.begin();
		while (i != items.end())
		{
			Entity* entity = *i;
			i = items.erase(i); //56006E..56008F releases the node before Destroy.
			entity->Destroy(); //560091, deferred state transition, not virtual deletion.
		}
	}
}
