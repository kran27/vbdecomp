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
	cell AMX_NATIVE_CALL NativeGetGlobal(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,1,"GetGlobal")) return 0;
		char variable[256];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[1],variable,256))
		{
			LogScriptError(vm,"GetGlobal() was unable to retrieve variable name.");
			return 0;
		}
		JE::IWorld* world=JE::g_World;
		return static_cast<cell>(world->GetGlobalParameters()->GetInteger(std::string(variable),std::string("")));
	}


	cell AMX_NATIVE_CALL NativeIncGlobal(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,1,"IncGlobal")) return 0;
		char variable[256];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[1],variable,256))
		{
			LogScriptError(vm,"IncGlobal() was unable to retrieve variable name.");
			return 0;
		}
		JE::IWorld* world=JE::g_World;
		DWORD bits=static_cast<DWORD>(world->GetGlobalParameters()->GetInteger(std::string(variable),std::string("")))+1;
		LONG value=static_cast<LONG>(bits); //Original INC wraps the full32-bit value.
		static_cast<JE::Server*>(g_pNetworkInstance->server)->SetGlobal(std::string(variable),value);
		return static_cast<cell>(value);
	}


	cell AMX_NATIVE_CALL NativeDecGlobal(AMX* vm, cell* parameters)
	{
		//The original5C8360 deliberately shares the IncGlobal diagnostic strings.
		if (!VerifyScriptParameters(vm,parameters,1,"IncGlobal")) return 0;
		char variable[256];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[1],variable,256))
		{
			LogScriptError(vm,"IncGlobal() was unable to retrieve variable name.");
			return 0;
		}
		JE::IWorld* world=JE::g_World;
		DWORD bits=static_cast<DWORD>(world->GetGlobalParameters()->GetInteger(std::string(variable),std::string("")))-1;
		LONG value=static_cast<LONG>(bits);
		static_cast<JE::Server*>(g_pNetworkInstance->server)->SetGlobal(std::string(variable),value);
		return static_cast<cell>(value);
	}

}
