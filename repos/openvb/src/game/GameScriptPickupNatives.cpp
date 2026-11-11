#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/ActionPickUp.hpp"
#include "entity/Entity.hpp"
#include "../startup/NetworkComponent.hpp"
#include "entity/World.hpp"
#include "network/EngineServer.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativePickupItem(AMX* vm,cell* parameters) //5C9380.
	{
		if (!VerifyScriptParameters(vm,parameters,2,"PickupItem")) return 0;
		JE::Entity* grabber;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&grabber,1))
		{
			LogScriptError(vm,"PickupItem() was unable to find the grabber entity.");
			return 0;
		}
		JE::Entity* item;
		if (!ResolveScriptEntity(vm,DWORD(parameters[2]),&item,1))
		{
			LogScriptError(vm,"PickupItem() was unable to find the target entity.");
			return 0;
		}
		if (item->inventoryOwnerID)
		{
			LogScriptError(vm,"PickupItem() cannot pick up item that already has an owner.");
			return 0;
		}
		JE::Server* server=static_cast<JE::Server*>(g_pNetworkInstance->server);
		server->RemoveEntityFromMap(item);
		//5C9439 reloads the actual Server after map/script callbacks.
		server=static_cast<JE::Server*>(g_pNetworkInstance->server);
		server->AddItemToInventory(grabber,item);
		return 1;
	}

	cell AMX_NATIVE_CALL NativeActionPickupItem(AMX* vm,cell* parameters) //5C9460.
	{
		if (!VerifyScriptParameters(vm,parameters,2,"A_PickupItem")) return 0;
		JE::Entity* grabber;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&grabber,1))
		{
			LogScriptError(vm,"A_PickupItem() was unable to find the grabber entity.");
			return 0;
		}
		JE::Entity* item;
		if (!ResolveScriptEntity(vm,DWORD(parameters[2]),&item,1))
		{
			LogScriptError(vm,"A_PickupItem() was unable to find the target entity.");
			return 0;
		}
		JE::ActionPickUp* action=static_cast<JE::ActionPickUp*>(JE::GetWorldInstance()->NewAction(3));
		action->SetTarget(item);
		grabber->QueueAction(action);
		return 1;
	}
}
