#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <cstring>
#include <stdexcept>

namespace F3
{
	cell AMX_NATIVE_CALL NativeIsEntityInArea(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "IsEntityInArea")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "IsEntityInArea() is unable to find the source entity.");
			return 0;
		}
		char area[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[2], area, sizeof(area)))
		{
			LogScriptError(vm, "IsEntityInArea() is unable to retrieve area name.");
			return 0;
		}
		//5C7E98 projects the nullable shared interface, then requires both owners.
		JE::IWorld* world = JE::GetWorldInstance();
		DWORD mapID = entity->mapHandle;
		JE::Map* map = world->GetMap(mapID);
		//The actual test is a case-insensitive prefix of the logical resource
		//name, not a trigger polygon, complete-name comparison or geometry name.
		return ::_strnicmp(area, map->resourceName.c_str(), std::strlen(area))==0;
	}
}
