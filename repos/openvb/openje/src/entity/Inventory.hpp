#pragma once

#include "Entity.hpp"
#include <list>

namespace JE
{
	class Inventory
	{
		friend class Entity;
	protected:
		virtual ~Inventory(); //560020; deleting wrapper5600E0, original slot00.
	public:
		virtual void Add(Entity* entity); // 0x560100; transfer before append/callback/spatial removal.
		virtual void Remove(Entity* entity); // 0x55FEE0; unlink before callback.
		virtual BYTE ContainsID(DWORD id); //55FD60, original+0C; actual Entity+1DC identity.
		virtual BYTE Contains(Entity* entity); //55FD30, original+10; pointer identity.
		virtual BYTE ContainsResource(const std::string& name); //55FF40,+14; exact instance-name string at+1C0.
		virtual Entity* FindResource(const std::string& name); //55FDA0,+18; first case-insensitive match.
		virtual DWORD CountResource(const std::string& name); //55FE20,+1C; count case-insensitive resource names.
		virtual void PackState(Net::Packet& packet); //Original+20:5A9320; game469370 jumps here.
		virtual void UnpackState(Net::Packet& packet); //Original+24:5A9320; caller54E944 passes the input packet.
		virtual BYTE GetEquippedSlots(Entity* entity, std::list<DWORD>& slots); //436230,+28; returns0/no writes.
		virtual BYTE IsEquipped(Entity* entity); //591690,+2C; base returns0.
		Entity* FindByID(DWORD id); //55FEA0, first nonnull item with matching nonzero ID.
		DWORD ownerEntityID;
		std::list<Entity*> items;
	protected:
		Inventory(); // 0x55FFF0.
		virtual void OnAdd(Entity* entity); //55FCD0,+30; publish inventory owner then SetMap.
		virtual void OnRemove(Entity* entity); //55FD10,+34; clear owner then map.
		//560020 unlinks each item before Entity::Destroy551B60; actual Entity
		//state/update ownership handles its later OnDestroy and retirement.
	};
}
