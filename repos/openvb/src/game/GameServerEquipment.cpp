#include "startup/GameServer.hpp"
#include "GameInventory.hpp"

namespace F3
{
	void GameServer::RemoveItemFromInventory(JE::Entity* owner,JE::Entity* item)
	{
		GameInventory* inventory=dynamic_cast<GameInventory*>(owner->inventory);
		if (inventory) inventory->Remove(item);
		PublishInventoryRemoval(owner->entityID,item->entityID,0);
	}
	BYTE GameServer::EquipItem(JE::Entity* owner,JE::Entity* item,DWORD slot,BYTE notify)
	{
		DWORD source=item->inventoryOwnerID; //46EBDB captures before the virtual inventory mutation.
		GameInventory* inventory=static_cast<GameInventory*>(owner->inventory);
		BYTE result=inventory->Equip(item,slot,notify);
		if (result) SendEquippedItem(owner->entityID,item->entityID,source,slot,0);
		return result;
	}
	void GameServer::UnequipItem(JE::Entity* owner,JE::Entity* item)
	{
		GameInventory* inventory=static_cast<GameInventory*>(owner->inventory);
		inventory->Unequip(item);
		SendUnequippedItem(owner->entityID,item->entityID,0);
	}
}
