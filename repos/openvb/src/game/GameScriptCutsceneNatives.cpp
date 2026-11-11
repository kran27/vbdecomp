#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/World.hpp"
#include "network/EngineServer.hpp"
#include "startup/NetworkComponent.hpp"
#include <stdexcept>

namespace F3
{
	cell AMX_NATIVE_CALL NativeStartCutscene(AMX* vm, cell* parameters) //5C9520.
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "StartCutscene")) return 0;
		void* owner;
		if (amx_GetUserData(vm, 0x464C534D, &owner))
		{
			LogScriptError(vm, "StartCutscene() was unable to resolve user ID. (internal error)");
			return 0;
		}
		DWORD mapID = ResolveScriptMapID(reinterpret_cast<DWORD>(owner));
		if (!mapID)
		{
			LogScriptError(vm, "StartCutscene() was unable to resolve the map.");
			return 0;
		}
		static_cast<JE::Server*>(g_pNetworkInstance->server)->StartCutscene(DWORD(parameters[1]), mapID);
		return 1;
	}
	cell AMX_NATIVE_CALL NativeStopCutscene(AMX* vm, cell* parameters) //5C95B0.
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "StopCutscene")) return 0;
		DWORD id = JE::GetWorldInstance()->GetCutsceneID();
		static_cast<JE::Server*>(g_pNetworkInstance->server)->StopCutscene();
		return cell(id);
	}
	cell AMX_NATIVE_CALL NativeIsInCutscene(AMX* vm, cell* parameters) //5C9610.
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "IsInCutscene")) return 0;
		return cell(JE::GetWorldInstance()->GetCutsceneID());
	}
}
