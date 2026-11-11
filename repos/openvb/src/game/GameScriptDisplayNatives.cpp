#include <winsock2.h>
#include "game/GameScriptNatives.hpp"
#include "game/FeedbackInterface.hpp"
#include "game/GameInterfaces.hpp"
#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "startup/EngineStringSystem.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeDisplayStringRef(AMX* vm, cell* parameters) //45A410.
	{
		if (!VerifyScriptParameters(vm, parameters, 4, "DisplayStringRef")) return 0;
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		if (!client)
		{
			LogScriptError(vm, "DisplayStringRef() couldn't find client.");
			return 0;
		}
		GameInterfaces* interfaces = static_cast<GameInterfaces*>(client->child0xA0);
		if (!interfaces)
		{
			LogScriptError(vm, "DisplayStringRef() couldn't find interfaces.");
			return 0;
		}
		FeedbackInterface* feedback = interfaces->feedback;
		if (!feedback)
		{
			LogScriptError(vm, "DisplayStringRef() couldn't find feedback interface.");
			return 0;
		}
		std::string text;
		if (!GetString(DWORD(parameters[1]), text)) //56C280; owner checks precede lookup.
		{
			LogScriptError(vm, "DisplayStringRef() couldn't find string ref.");
			return 0;
		}
		feedback->AddMessage(text, DWORD(parameters[2]), DWORD(parameters[3]),
			DWORD(parameters[4]), std::string("")); //434B70; original empty-literal constructor.
		return 1; //Explicit MOV EAX,1 at45A569, after local string destruction.
	}
}
