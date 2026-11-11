#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptEnvironment.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeAttachVEG(AMX* vm,cell* parameters) //5C29B0.
	{
		if (!VerifyScriptParameters(vm,parameters,2,"AttachVEG")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"AttachVEG() is unable to find the requested entity.");
			return 0;
		}
		char name[260];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[2],name,sizeof(name)))
		{
			LogScriptError(vm,"AttachVEG() is unable to retrieve VEG name.");
			return 0;
		}
		JE::Server* server=static_cast<JE::Server*>(g_pNetworkInstance->server);
		return cell(server->AttachVisualEffect(entity,name));
	}
	cell AMX_NATIVE_CALL NativeRemoveVEG(AMX* vm,cell* parameters) //5C2A80.
	{
		if (!VerifyScriptParameters(vm,parameters,2,"RemoveVEG")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"RemoveVEG() is unable to find the requested entity.");
			return 0;
		}
		static_cast<JE::Server*>(g_pNetworkInstance->server)->RemoveVisualEffect(entity,DWORD(parameters[2]),0);
		return 1;
	}
	cell AMX_NATIVE_CALL NativeGetVEGID(AMX* vm,cell* parameters) //5C2AF0.
	{
		if (!VerifyScriptParameters(vm,parameters,2,"GetVEGID")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"GetVEGID() is unable to find the requested entity.");
			return 0;
		}
		char name[260];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[2],name,sizeof(name)))
		{
			LogScriptError(vm,"GetVEGID() is unable to retrieve VEG name.");
			return 0;
		}
		return cell(entity->GetVisualEffectID(name));
	}
}
