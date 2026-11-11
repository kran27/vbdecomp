#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include <new>

namespace F3
{
	cell AMX_NATIVE_CALL NativeSendMessage(AMX* vm, cell* parameters)
	{
		LONG count;
		if (!VerifyMinimumScriptParameters(vm, parameters, 2, "SendMessage", &count)) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "SendMessage() was unable to retrieve the 'from' id.");
			return 0;
		}
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "SendMessage() was unable to find the requested entity.");
			return 0;
		}
		cell* arguments = new cell[count - 2];
		for (LONG i = 0; i < count - 2; ++i)
		{
			cell* source;
			if (amx_GetAddr(vm, parameters[i + 3], &source))
			{
				LogScriptError(vm, "SendMessage() was unable to resolve address of parameter %d.", i + 2);
				return 0; //Original temporary allocation leak on address failure.
			}
			arguments[i] = *source;
		}
		entity->EnqueueMessage(static_cast<DWORD>(reinterpret_cast<UINT_PTR>(owner)),
			DWORD(parameters[2]), arguments, count - 2);
		delete[] arguments;
		return 1;
	}

	cell AMX_NATIVE_CALL NativeMessageArgCount(AMX* vm, cell* parameters)
	{
		(void)parameters; //Original5C5540 does not verify an argument count.
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "MessageArgCount() was unable to retrieve the source entity id.");
			return 0;
		}
		DWORD id = static_cast<DWORD>(reinterpret_cast<UINT_PTR>(owner));
		if (id & 0xE0000000)
		{
			LogScriptError(vm, "MessageArgCount() source entity id is not an entity.");
			return 0;
		}
		JE::Entity* entity = JE::GetEntityPtr(id);
		if (!entity)
		{
			LogScriptError(vm, "MessageArgCount() unable to retrieve source entity.");
			return 0;
		}
		return entity->MessageArgCount();
	}

	cell AMX_NATIVE_CALL NativeMessageGetArg(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "MessageGetArg")) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "MessageGetArg() was unable to retrieve the source entity id.");
			return 0;
		}
		DWORD id = static_cast<DWORD>(reinterpret_cast<UINT_PTR>(owner));
		if (id & 0xE0000000)
		{
			LogScriptError(vm, "MessageGetArg() source entity id is not an entity.");
			return 0;
		}
		JE::Entity* entity = JE::GetEntityPtr(id);
		if (!entity)
		{
			LogScriptError(vm, "MessageGetArg() unable to retrieve source entity.");
			return 0;
		}
		return entity->MessageGetArg(parameters[1]);
	}
}
