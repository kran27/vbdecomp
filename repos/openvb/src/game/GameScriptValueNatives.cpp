#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeGetValue(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "GetValue")) return 0;
		JE::Entity* entity = NULL;
		if (!ResolveScriptEntity(vm, parameters[1], &entity, 1))
		{
			LogScriptError(vm, "GetValue() was unable to find the requested entity.");
			return 0;
		}
		return static_cast<cell>(entity->GetScriptValue(parameters[2]));
	}

	cell AMX_NATIVE_CALL NativeSetValue(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 3, "SetValue")) return 0;
		JE::Entity* entity = NULL;
		if (!ResolveScriptEntity(vm, parameters[1], &entity, 1))
		{
			LogScriptError(vm, "SetValue() was unable to find the requested entity.");
			return 0;
		}
		entity->SetScriptValue(parameters[2], parameters[3]);
		return static_cast<cell>(entity->GetScriptValue(parameters[2]));
	}

	cell AMX_NATIVE_CALL NativeIncValue(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "IncValue")) return 0;
		JE::Entity* entity = NULL;
		if (!ResolveScriptEntity(vm, parameters[1], &entity, 1))
		{
			LogScriptError(vm, "IncValue() was unable to find the requested entity.");
			return 0;
		}
		DWORD value = entity->GetScriptValue(parameters[2]) + 1;
		entity->SetScriptValue(parameters[2], value);
		return static_cast<cell>(value);
	}

	cell AMX_NATIVE_CALL NativeDecValue(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "DecValue")) return 0;
		JE::Entity* entity = NULL;
		if (!ResolveScriptEntity(vm, parameters[1], &entity, 1))
		{
			LogScriptError(vm, "DecValue() was unable to find the requested entity.");
			return 0;
		}
		DWORD value = entity->GetScriptValue(parameters[2]) - 1;
		entity->SetScriptValue(parameters[2], value);
		return static_cast<cell>(value);
	}
}
