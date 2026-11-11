#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "startup/GameServer.hpp"
#include "startup/NetworkComponent.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeOpenTask(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "OpenTask")) return 0;
		static_cast<GameServer*>(g_pNetworkInstance->server)->OpenTask( DWORD(parameters[1]), DWORD(parameters[2]));
		return 1;
	}
	cell AMX_NATIVE_CALL NativeCloseTask(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "CloseTask")) return 0;
		static_cast<GameServer*>(g_pNetworkInstance->server)->CloseTask( DWORD(parameters[1]), DWORD(parameters[2]));
		return 1;
	}
	cell AMX_NATIVE_CALL NativeAddEntry(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "AddEntry")) return 0;
		static_cast<GameServer*>(g_pNetworkInstance->server)->AddEntry( DWORD(parameters[1]), DWORD(parameters[2]));
		return 1;
	}
}
