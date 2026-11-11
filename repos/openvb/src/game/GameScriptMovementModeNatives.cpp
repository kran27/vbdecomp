#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "startup/GameServer.hpp"
#include "startup/NetworkComponent.hpp"
#include "scripting/ScriptEnvironment.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeSetMoveMode(AMX* vm,cell* parameters) //45A140.
	{
		if (!VerifyScriptParameters(vm,parameters,3,"SetMoveMode")) return 0;
		float speed = 0.0f;
		JE::ScriptServices::ReadNativeFloat(vm, parameters[3], speed);
		GameServer* server = static_cast<GameServer*>(g_pNetworkInstance->server);
		if (server) server->SetMoveMode(DWORD(parameters[1]), DWORD(parameters[2]), speed);
		return 1; //45A1A0, including absent server or unmatched entity.
	}
}
