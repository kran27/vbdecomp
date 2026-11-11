#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "startup/GameServer.hpp"
#include "startup/NetworkComponent.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeHideInterfaces(AMX* vm, cell* parameters) //45A100.
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "HideInterfaces")) return 0;
		static_cast<GameServer*>(g_pNetworkInstance->server)->HideInterfaces();
		return 1;
	}
}
