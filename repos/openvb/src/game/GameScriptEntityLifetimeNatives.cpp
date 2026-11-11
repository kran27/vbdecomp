#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeDestroyEntity(AMX* vm, cell* parameters) //5CA5F0.
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "DestroyEntity")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "DestroyEntity() could not find the requested entity.");
			return 0;
		}
		entity->Destroy(); //State transition; World retires it on a later pass.
		return 1;
	}
}
