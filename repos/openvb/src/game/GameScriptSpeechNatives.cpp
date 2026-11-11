#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/World.hpp"
#include "entity/ActionFloatingSpeechText.hpp"
#include "network/EngineServer.hpp"
#include "../startup/EngineStringSystem.hpp"
#include "../startup/NetworkComponent.hpp"
#include <stdexcept>

namespace F3
{
	cell AMX_NATIVE_CALL NativeImmediateFloatingSpeechText(AMX* vm,cell* parameters) //5C25E0.
	{
		if (!VerifyScriptParameters(vm,parameters,6,"AddFloatingSpeechText")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"AddFloatingSpeechText() is unable to find the requested entity.");
			return 0;
		}
		DWORD stringReference=DWORD(parameters[2]),duration=DWORD(parameters[3]);
		JE::Server* server=static_cast<JE::Server*>(g_pNetworkInstance->server);
		server->FloatingSpeechText(entity->entityID,stringReference,duration,
			DWORD(parameters[4]),DWORD(parameters[5]),DWORD(parameters[6]));
		if (!duration)
		{
			std::string text;
			GetString(stringReference,text); //The Server request precedes the optional lookup.
			return cell(static_cast<__int64>((DWORD(text.size())*0.1f+1.0f)*1000.0f));
		}
		return cell(duration);
	}
	cell AMX_NATIVE_CALL NativeFloatingSpeechText(AMX* vm,cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,6,"A_AddFloatingSpeechText")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"A_AddFloatingSpeechText() is unable to find the requested entity.");
			return 0;
		}
		DWORD stringReference=DWORD(parameters[2]),duration=DWORD(parameters[3]);
		DWORD red=DWORD(parameters[4]),green=DWORD(parameters[5]),blue=DWORD(parameters[6]);
		if (!JE::g_World) throw std::logic_error("FloatingSpeechText5C27A9: actual World factory required");
		JE::ActionFloatingSpeechText* action=static_cast<JE::ActionFloatingSpeechText*>(JE::g_World->NewAction(8));
		action->stringReference=stringReference;
		action->duration=duration;
		action->red=red;
		action->green=green;
		action->blue=blue;
		entity->QueueAction(action);
		if (duration) return cell(duration);
		std::string text;
		GetString(stringReference,text); //Queue publication precedes this separate56C280 lookup.
		return cell(JE::FloatingSpeechDuration(DWORD(text.size())));
	}
}
