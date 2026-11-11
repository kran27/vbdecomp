#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "../startup/NetworkComponent.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptEnvironment.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeAttachSound(AMX* vm, cell* parameters) //5C2860.
	{
		if (!VerifyScriptParameters(vm,parameters,3,"AttachSound")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"AttachSound() is unable to find the requested entity.");
			return 0;
		}
		char filename[260];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[2],filename,sizeof(filename)))
		{
			LogScriptError(vm,"AttachSound() is unable to retrieve sound filename.");
			return 0;
		}
		cell looping=parameters[3];
		JE::Server* server=static_cast<JE::Server*>(g_pNetworkInstance->server);
		return cell(server->AttachSound(entity,filename,looping!=0));
	}
	cell AMX_NATIVE_CALL NativeRemoveSound(AMX* vm, cell* parameters) //5C2940.
	{
		if (!VerifyScriptParameters(vm,parameters,2,"RemoveSound")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"RemoveSound() is unable to find the requested entity.");
			return 0;
		}
		static_cast<JE::Server*>(g_pNetworkInstance->server)->RemoveSound(entity,DWORD(parameters[2]),0);
		return 1; //Original returns1 even if Entity54F590 found no matching sound.
	}
}
