#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "GameEntity.hpp"
#include "../startup/NetworkComponent.hpp"
#include "../startup/GameServer.hpp"
#include "network/EngineServer.hpp"

namespace F3
{

	cell AMX_NATIVE_CALL NativeSetHP(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "SetHP")) return 0;
		JE::Entity* owner = NULL;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &owner, 1))
		{
			LogScriptError(vm, "SetHP() - Unable to resolve requested entity.");
			return 0;
		}
		GameEntity* entity = dynamic_cast<GameEntity*>(owner);
		if (!entity)
		{
			LogScriptError(vm, "SetHP() - Entity ID does not belong to a GameEntity.");
			return 0;
		}
		DWORD id = entity->entityID;
		GameServer* server = static_cast<GameServer*>(g_pNetworkInstance->server);
		return cell(server->SetEntityHealth(id, parameters[2]));
	}

	cell AMX_NATIVE_CALL NativeAddHP(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "AddHP")) return 0;
		JE::Entity* owner = NULL;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &owner, 1))
		{
			LogScriptError(vm, "AddHP() - Unable to resolve requested entity.");
			return 0;
		}
		GameEntity* entity = dynamic_cast<GameEntity*>(owner);
		if (!entity)
		{
			LogScriptError(vm, "AddHP() - Entity ID does not belong to a GameEntity.");
			return 0;
		}
		LONG health = static_cast<LONG>(DWORD(entity->currentHealth) + DWORD(parameters[2]));
		DWORD id = entity->entityID;
		GameServer* server = static_cast<GameServer*>(g_pNetworkInstance->server);
		return cell(server->SetEntityHealth(id, health));
	}
}
