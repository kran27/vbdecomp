#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "GameScriptMapContext.hpp"
#include "entity/Trigger.hpp"
#include "entity/Entity.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <cstring>

namespace F3
{
	cell AMX_NATIVE_CALL NativeSpawnEntity(AMX* vm, cell* parameters) //5CA650.
	{
		if (!VerifyScriptParameters(vm, parameters, 3, "SpawnEntity")) return 0;
		char resource[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], resource, sizeof(resource)))
		{
			LogScriptError(vm, "SpawnEntity() could not retreive the file name of the entity to spawn.");
			return 0;
		}
		D3DXVECTOR3 destination(0, 0, 0);
		if (!ReadScriptCells(vm, parameters[2], &destination, 3))
		{
			LogScriptError(vm, "SpawnEntity() could not retreive the destination vector.");
			return 0;
		}
		char scriptName[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[3], scriptName, sizeof(scriptName)))
		{
			LogScriptError(vm, "SpawnEntity() could not retreive the script name for the entity to spawn.");
			return 0;
		}
		void* owner;
		amx_GetUserData(vm, 0x464C534D, &owner); //5CA749 consumes its output even after failure.
		DWORD mapID = reinterpret_cast<DWORD>(owner);
		switch (mapID >> 29)
		{
		case 0:
			{
				JE::Entity* context = JE::GetEntityPtr(mapID);
				mapID = context ? context->mapHandle : 0;
				break;
			}
		case 1: break;
		case 2:
			{
				JE::Trigger* context = JE::FindTrigger(mapID);
				mapID = context ? context->mapID : 0;
				break;
			}
		default: mapID = 0;
		}
		if (!mapID)
		{
			LogScriptError(vm, "SpawnEntity() was unable to resolve the map.");
			return 0;
		}
		JE::Entity* entity = static_cast<JE::Server*>(g_pNetworkInstance->server)->CreateEntity(resource, 0, 0);
		entity->SetScriptName(scriptName); //Actual426510, assigns name without loading script.
		D3DXVECTOR3 rotation; //Native raw-word snapshot after the name assignment.
		std::memcpy(&rotation.x, &entity->rotation.x, sizeof(float));
		std::memcpy(&rotation.y, &entity->rotation.y, sizeof(float));
		std::memcpy(&rotation.z, &entity->rotation.z, sizeof(float));
		static_cast<JE::Server*>(g_pNetworkInstance->server)->MoveEntityToMap(entity, mapID, destination, rotation, 0);
		return 1; //Explicit5CA7FC. Earlier creation/publication is not rolled back on failure.
	}
}
