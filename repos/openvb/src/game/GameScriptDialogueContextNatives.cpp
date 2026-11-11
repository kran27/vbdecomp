#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include "../startup/NetworkComponent.hpp"

namespace F3
{
	namespace
	{
		JE::DialogueManager& Dialogue()
		{
			//Each original native reloads70BD90 -> Network.server -> Server+5064.
			JE::DialogueManager* manager=static_cast<JE::DialogueManager*>(
				static_cast<JE::Server*>(g_pNetworkInstance->server)->dialogue);
			return *manager;
		}
	}
	cell AMX_NATIVE_CALL NativeRegisterNodeText(AMX* vm,cell* parameters) //5C3980.
	{
		if (!VerifyScriptParameters(vm,parameters,1,"RegisterNodeText")) return 0;
		Dialogue().RegisterNodeText(DWORD(parameters[1]));
		return 1;
	}
	cell AMX_NATIVE_CALL NativeRegisterReply(AMX* vm,cell* parameters) //5C4060.
	{
		if (!VerifyScriptParameters(vm,parameters,4,"RegisterReply")) return 0;
		DWORD replyID=DWORD(parameters[1]),textID=DWORD(parameters[2]);
		char action[64],next[64];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[3],action,sizeof(action)))
		{
			LogScriptError(vm,"RegisterReply() was unable to retrieve the action function name.");
			return 0;
		}
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[4],next,sizeof(next)))
		{
			LogScriptError(vm,"RegisterReply() was unable to retrieve the next node function name.");
			return 0;
		}
		Dialogue().RegisterReply(replyID,textID,action,next);
		return 1;
	}
	cell AMX_NATIVE_CALL NativeSetNodeTextFunction(AMX* vm,cell* parameters) //5C41B0.
	{
		if (!VerifyScriptParameters(vm,parameters,1,"SetNodeTextFunction")) return 0;
		char name[64];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[1],name,sizeof(name)))
		{
			LogScriptError(vm,"SetNodeTextFunction() was unable to retrieve the node text function name.");
			return 0;
		}
		Dialogue().SetNodeTextFunction(name);
		return 1;
	}
	cell AMX_NATIVE_CALL NativeSetCurrentNodeName(AMX* vm,cell* parameters) //5C42A0.
	{
		if (!VerifyScriptParameters(vm,parameters,1,"SetCurrentNodeName")) return 0;
		char name[260];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[1],name,sizeof(name)))
		{
			LogScriptError(vm,"SetCurrentNodeName() was unable to retrieve the node name.");
			return 0;
		}
		Dialogue().SetCurrentNodeName(name);
		return 1;
	}
}
