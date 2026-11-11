#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeActivateEntity(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "ActivateEntity")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 0))
		{
			LogScriptError(vm, "ActivateEntity() was unable to find the requested entity.");
			return 0;
		}
		if (entity->renderEnabled)
		{
			LogScriptError(vm, "ActivateEntity() called on already active entity.");
			return 0;
		}
		//5C91EF writes only Entity+3A0. Existing server/client update, drawing
		//and object-query paths consume this shared byte. No new state/packet.
		entity->renderEnabled=1;
		return 1;
	}

	cell AMX_NATIVE_CALL NativeDeactivateEntity(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "DeactivateEntity")) return 0;
		JE::Entity* entity;
		//59AEF0's inactive diagnostic does not change its successful lookup
		//return. Keep requireActive1 before the separate already-inactive test.
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "DeactivateEntity() was unable to find the requested entity.");
			return 0;
		}
		if (!entity->renderEnabled)
		{
			LogScriptError(vm, "DeactivateEntity() called on already inactive entity.");
			return 0;
		}
		entity->renderEnabled=0; //5C926F, no callback or spatial/list removal.
		return 1;
	}
}
