#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <string>
#include <vector>

namespace F3
{
	cell AMX_NATIVE_CALL NativeQueueFunction(AMX* vm, cell* parameters) //5CA4F0.
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "QueueFunction")) return 0;
		//5CA535 is signed JL. The supplied length is only this admission test;
		//the actual Small string reader always receives the full260-byte buffer.
		if (static_cast<LONG>(parameters[2]) >= 260)
		{
			LogScriptError(vm, "QueueFunction - Name too long.");
			return 0;
		}
		char name[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], name, sizeof(name)))
		{
			LogScriptError(vm, "QueueFunction - Unable to retrieve function name string.");
			return 0;
		}
		void* queue = NULL;
		amx_GetUserData(vm, 0x45555146, &queue); //FQUE; original ignores API status.
		if (!queue)
		{
			LogScriptError(vm, "QueueFunction - Error in trying to retrieve function queue.");
			return 0;
		}
		static_cast<std::vector<std::string>*>(queue)->push_back(std::string(name));
		return 1; //Explicit5CA5CF despite the retained void annotation.
	}

	cell AMX_NATIVE_CALL NativeClearFunctionQueue(AMX* vm, cell* parameters) //5C9F80.
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "ClearFunctionQueue")) return 0;
		void* queue = NULL;
		amx_GetUserData(vm, 0x45555146, &queue);
		if (!queue)
		{
			LogScriptError(vm, "ClearFunctionQueue - Error in trying to retrieve function queue.");
			return 0;
		}
		static_cast<std::vector<std::string>*>(queue)->clear();
		return 1;
	}
}
