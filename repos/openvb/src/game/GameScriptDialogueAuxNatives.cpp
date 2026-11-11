#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include "../startup/EngineStringSystem.hpp"
#include "../startup/NetworkComponent.hpp"
#include <cstdlib>
#include <string>

namespace F3
{
	namespace
	{
		JE::DialogueManager& Dialogue()
		{
			JE::DialogueManager* manager = static_cast<JE::DialogueManager*>(
				static_cast<JE::Server*>(g_pNetworkInstance->server)->dialogue);
			return *manager;
		}

	}

	cell AMX_NATIVE_CALL NativeWasLastNodeName(AMX* vm, cell* parameters) //5C4390.
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "WasLastNodeName")) return 0;
		char name[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], name, sizeof(name)))
		{
			LogScriptError(vm, "WasLastNodeName() was unable to retrieve the node name.");
			return 0;
		}
		BYTE result;
		{
			std::string nodeName(name);
			result = Dialogue().WasLastNodeName(nodeName);
		}
		return result != 0;
	}

	cell AMX_NATIVE_CALL NativePCSpeaker(AMX* vm, cell* parameters) //5C3A50.
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "PCSpeaker")) return 0;
		void* owner;
		amx_GetUserData(vm, 0x464C534D, &owner); //5C3A7A ignores status; failure leaves owner unwritten.
		if (!JE::GetEntityPtr(static_cast<DWORD>(reinterpret_cast<UINT_PTR>(owner))))
		{
			LogScriptError(vm, "PCSpeaker() - Unable to locate the entity that owns this active dialogue.");
			return 0;
		}
		DWORD speaker = Dialogue().GetPCSpeaker(static_cast<DWORD>(reinterpret_cast<UINT_PTR>(owner)));
		if (!speaker)
		{
			LogScriptError(vm, "PCSpeaker() - Could not find a dialogue owned by this entity.");
			return 0;
		}
		return cell(speaker);
	}

	cell AMX_NATIVE_CALL NativeSetTokenStrref(AMX* vm, cell* parameters) //5C3AE0.
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "SetTokenStrref")) return 0;
		char name[65];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], name, sizeof(name)))
		{
			LogScriptError(vm, "NativeSetTokenStrref() was unable to retrieve the token name.");
			return 0;
		}
		g_EngineStringSystem.SetTokenValue(name, parameters[2]); //56B940 ->5AD460.
		return parameters[2];
	}

	cell AMX_NATIVE_CALL NativeSetTokenInt(AMX* vm, cell* parameters) //5C3B80.
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "SetTokenInt")) return 0;
		char name[65];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], name, sizeof(name)))
		{
			LogScriptError(vm, "NativeSetTokenStrref() was unable to retrieve the token name.");
			return 0;
		}
		char value[32];
		g_EngineStringSystem.SetTokenText(name, ::_itoa(parameters[2], value, 10));
		return parameters[2];
	}
}
