#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeIsInCombat(AMX* vm, cell* parameters) //5C7B30.
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "IsInCombat")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "IsInCombat() is unable to find the source entity.");
			return 0;
		}
		return entity->state == 5;
	}
}
