#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "../startup/NetworkComponent.hpp"
#include "network/EngineServer.hpp"
#include "entity/World.hpp"
#include "entity/EntityEnvironment.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <stdexcept>

namespace F3
{
	cell AMX_NATIVE_CALL NativeSetGlobal(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,2,"SetGlobal")) return 0;
		char variable[256];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[1],variable,256))
		{
			LogScriptError(vm,"SetGlobal() was unable to retrieve variable name.");
			return 0;
		}
		static_cast<JE::Server*>(g_pNetworkInstance->server)->SetGlobal(
			std::string(variable),static_cast<LONG>(parameters[2]));
		return 1;
	}

}
