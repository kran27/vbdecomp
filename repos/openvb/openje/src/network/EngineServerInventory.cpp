#include "EngineServer.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/Entity.hpp"
#include "entity/Inventory.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "model/CommonException.hpp"
#include <cstring>

namespace JE
{
	namespace
	{
		DWORD FloatWord(float value)
		{
			DWORD bits;
			std::memcpy(&bits,&value,sizeof(bits));
			return bits;
		}
	}

	void Server::AddItemToInventory(Entity* owner,Entity* item) //555640 ->553850.
	{
		owner->inventory->Add(item);
		// Capture the live IDs after mutation, before the actual sender constructs its Packet.
		PublishInventoryTransfer(owner->entityID,item->entityID,item->inventoryOwnerID,0);
	}

	void Server::RemoveItemFromInventory(Entity* owner,Entity* item) //555680 ->553B00.
	{
		owner->inventory->Remove(item);
		PublishInventoryRemoval(owner->entityID,item->entityID,0);
	}

	void Server::RemoveEntityFromMap(Entity* entity) //5556C0 ->56E500 ->553C30.
	{
		//The original captures +1EC before evaluating the server-mode guard.
		DWORD mapID=entity->mapHandle;
		if (!(F3::g_pNetworkInstance->GetMode()&1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		Map* map=world->GetMap(mapID);
		if (map)
		{
			map->DetachEntity(entity);
			SendEntityMapRemoval(entity->entityID); //55572C, live ID after detach callbacks.
		}
	}
	void Server::SendEntityMapRemoval(DWORD entityID) //553C30.
	{
		Net::Packet notification;
		notification.field0x30=13;
		notification.WriteDword(entityID);
		Broadcast(notification,1);
	}

	void Server::MoveEntityToMap(Entity* entity,DWORD mapID,const D3DXVECTOR3& position,
		const D3DXVECTOR3& rotation,BYTE flag) //5562F0.
	{
		if (!(F3::g_pNetworkInstance->GetMode()&1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		Map* map=world->GetMap(mapID);
		AddEntityToMap(entity,map,position,rotation,flag);
	}

	void Server::AddEntityToMap(Entity* entity,Map* map,const D3DXVECTOR3& position,
		const D3DXVECTOR3& rotation,BYTE flag) //5555D0 ->553990.
	{
		entity->ChangePosition(position);
		map->AttachEntity(entity);
		entity->SetPosition(position);
		entity->SetRotation(rotation);
		if (entity->enabled) entity->SnapToFloor();
		SendEntityMapAddition(entity->entityID,map->idB4,flag,entity->position);
	}

	void Server::SendEntityMapAddition(DWORD entityID,DWORD mapID,BYTE flag,
		const D3DXVECTOR3& position) //553990.
	{
		Net::Packet notification;
		notification.field0x30=12;
		notification.WriteDword(entityID);
		notification.WriteDword(mapID);
		notification.WriteByte(flag);
		// Reload each live coordinate only after the preceding packet write.
		notification.WriteDword(FloatWord(position.x));
		notification.WriteDword(FloatWord(position.y));
		notification.WriteDword(FloatWord(position.z));
		Broadcast(notification,1);
	}
}
