#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeEnableDialogue(AMX* vm, cell* parameters) //5C9280.
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "EnableDialogue")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "EnableDialogue() was unable to find the requested entity.");
			return 0;
		}
		if (entity->dialogueEnabled)
		{
			LogScriptError(vm, "EnableDialogue() called on an entity whose dialogue is already enabled.");
			return 0;
		}
		entity->dialogueEnabled = 1; //5C92EF writes the single raw byte only.
		return 1;
	}

	cell AMX_NATIVE_CALL NativeDisableDialogue(AMX* vm, cell* parameters) //5C9300.
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "DisableDialogue")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "DisableDialogue() was unable to find the requested entity.");
			return 0;
		}
		if (!entity->dialogueEnabled)
		{
			LogScriptError(vm, "DisableDialogue() called on an entity whose dialogue is already disabled.");
			return 0;
		}
		entity->dialogueEnabled = 0; //5C936F; existing dialogue contexts remain unchanged.
		return 1;
	}
}
