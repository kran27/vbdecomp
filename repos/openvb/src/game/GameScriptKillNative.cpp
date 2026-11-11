#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeKill(AMX* vm,cell* parameters) //5C8E70.
	{
		if (!VerifyScriptParameters(vm,parameters,1,"Kill")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"Kill() was unable to find the requested entity.");
			return 0;
		}
		entity->ClearActions();
		entity->QueueAction(JE::GetWorldInstance()->NewAction(0));
		return 1;
	}
}
