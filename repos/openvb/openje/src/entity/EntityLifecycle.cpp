#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineServer.hpp"
#include "Entity.hpp"
#include "Inventory.hpp"
#include "World.hpp"
#include "Map.hpp"
#include "EntityEnvironment.hpp"
#include "SpatialFootprint.hpp"
#include "model/CommonException.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <new>
#include <stdexcept>

namespace JE
{
	DWORD Entity::AllocateEntityID()
	{
		IWorldClient* worldClient = g_World;
		IWorld* world = worldClient;
		return world->AllocateEntityID(); //551909, original required IWorld+24 dispatch.
	}
	Inventory* Entity::CreateInventory() { return new Inventory; }
	std::string Entity::PlayerResourceName()
	{
		return GetEntityEnvironment().PlayerResourceName();
	}
	void Entity::PublishPlayerID(DWORD id)
	{
		GetEntityEnvironment().PublishPlayerID(id);
	}
	void Entity::SetDisplayName(const std::string& value)
	{
		if (_stricmp(displayName.c_str(), value.c_str())) displayName = value;
	}
	void Entity::SetClientOwner(DWORD value)
	{
		if (visibilityOwnerFlag == value) return;
		if (!visibilityOwnerFlag && value)
		{
			ClearActions();
			IWorld* world = g_World;
			QueueAction(world->NewAction(1));
			if (mapHandle)
			{
				IWorld* mapWorld = g_World;
				mapWorld->GetMap(mapHandle)->visibilityOwners++;
			}
		}
		else if (visibilityOwnerFlag && !value)
		{
			DWORD mapID = mapHandle;
			if (mapID) GetWorldInstance()->GetMap(mapID)->visibilityOwners--;
		}
		DWORD id = entityID;
		visibilityOwnerFlag = value;
		Client* client = static_cast<Client*>(F3::g_pNetworkInstance->client);
		if (client->field0xE0 == id) client->field0xE0 = 0;
	}
	DWORD Entity::CreateNetworkMode()
	{
		return GetEntityEnvironment().NetworkMode();
	}
	RLEPrimitive<Entity*>* Entity::CreateSpatialFootprint()
	{
		return new RLECircle<Entity*>(1, 0, 0, this);
	}
	void Entity::OnCreated() {} // Original shared404B70.
	void Entity::OnModelLoaded() {} // Original shared404B70.
	DWORD Entity::GetUpdateInterval() const { return updateInterval; } // Original408080.
	void Entity::SetUpdateInterval(DWORD value)
	{
		updateInterval = value;
		if (NeedsUpdate()) AddToUpdateQueue();
		else RemoveFromUpdateQueue();
	}
	BYTE Entity::IsPeriodic() { return GetUpdateInterval() > 0 ? 1 : 0; }

	BYTE Entity::NeedsUpdate()
	{
		BOOL result = IsPeriodic() || state == 1 || state == 3 || state == 2 || state == 6 || !effects.empty() || !actions.empty();
		return BYTE(result);
	}

	BYTE Entity::UpdateDue(DWORD time)
	{
		if (state == 1 || state == 2 || state == 3 || state == 6) return 1;
		if (!IsPeriodic() && (!effects.empty() || !actions.empty())) return 1;
		return IsPeriodic() && nextUpdateTime <= time;
	}

	void Entity::ChangeState(DWORD value)
	{
		if (!state && value != 1 && value != 3)
			throw ::Common::Exception("Invalid state change in Entity::ChangeState().");
		if ((state == 1 || state == 3) && value != 4)
			throw ::Common::Exception("Invalid state change in Entity::ChangeState().");
		if (state == 2 && value != 8)
			throw ::Common::Exception("Invalid state change in Entity::ChangeState().");
		if (state == 4 && value != 5 && value != 6 && value != 2)
			throw ::Common::Exception("Invalid state change in Entity::ChangeState().");
		if (state == 5 && value != 4 && value != 6 && value != 2)
			throw ::Common::Exception("Invalid state change in Entity::ChangeState().");
		if (state == 6 && value != 7 && value != 2)
			throw ::Common::Exception("Invalid state change in Entity::ChangeState().");
		if (state == 7 && value != 2)
			throw ::Common::Exception("Invalid state change in Entity::ChangeState().");
		state = value;
		if (NeedsUpdate()) AddToUpdateQueue();
		else RemoveFromUpdateQueue();
	}


}
