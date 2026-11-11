#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "entity/Map.hpp"
#include "entity/Trigger.hpp"
#include "entity/World.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeGetAreaRestStatus(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "GetAreaRestStatus")) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "GetAreaRestStatus() was unable to retrieve 'myself' id.");
			return 0;
		}
		DWORD id = reinterpret_cast<DWORD>(owner), mapID;
		switch (id >> 29)
		{
		case 0:
			{
				JE::Entity* entity = JE::GetEntityPtr(id);
				mapID = entity ? entity->mapHandle : 0;
				break;
			}
		case 1: mapID = id; break;
		case 2:
			{
				JE::Trigger* trigger = JE::FindTrigger(id);
				mapID = trigger ? trigger->mapID : 0;
				break;
			}
		default: mapID = 0;
		}
		if (!mapID)
		{
			LogScriptError(vm, "GetAreaRestStatus() was unable to resolve the map.");
			return 0;
		}
		JE::IWorld* world = JE::GetWorldInstance();
		JE::Map* map = world->GetMap(mapID);
		return map->flag1D8 == 1;
	}
	cell AMX_NATIVE_CALL NativeSetAreaRestStatus(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "SetAreaRestStatus")) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "SetAreaRestStatus() was unable to retrieve 'myself' id.");
			return 0;
		}
		DWORD id = reinterpret_cast<DWORD>(owner), mapID;
		switch (id >> 29)
		{
		case 0:
			{
				JE::Entity* entity = JE::GetEntityPtr(id);
				mapID = entity ? entity->mapHandle : 0;
				break;
			}
		case 1: mapID = id; break;
		case 2:
			{
				JE::Trigger* trigger = JE::FindTrigger(id);
				mapID = trigger ? trigger->mapID : 0;
				break;
			}
		default: mapID = 0;
		}
		if (!mapID)
		{
			LogScriptError(vm, "SetAreaRestStatus() was unable to resolve the map.");
			return 0;
		}
		JE::IWorld* world = JE::GetWorldInstance();
		BYTE enabled = parameters[1] != 0;
		JE::Map* map = world->GetMap(mapID);
		map->flag1D8 = enabled;
		return 1;
	}
}
