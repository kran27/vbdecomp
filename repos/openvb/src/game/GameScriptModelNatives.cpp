#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <cstdlib>
#include <new>

namespace JE { namespace ScriptServices { BYTE ReadNativeStringLength(AMX*, cell, int&); } }

namespace F3
{
	cell AMX_NATIVE_CALL NativeSetModel(AMX* vm, cell* parameters) //5C78A0.
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "SetModel")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "SetModel() is unable to find the source entity.");
			return 0;
		}
		int length;
		if (!JE::ScriptServices::ReadNativeStringLength(vm, parameters[2], length))
		{
			LogScriptError(vm, "SetModel() is unable to determine the length of the root name.");
			return 0;
		}
		char* name = static_cast<char*>(::operator new(length + 1));
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[2], name, DWORD(length + 1)))
		{
			//5C7948 returns without freeing the original temporary allocation.
			LogScriptError(vm, "SetModel() is unable to retrieve the root name.");
			return 0;
		}
		entity->SetModel(name, 0); //Direct54DC50 call: no Server setter or packet.
		::operator delete[](name); //Original scalar-new / array-delete pairing.
		return 1;
	}
}
