#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include "../startup/NetworkComponent.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeFloatingDamageText(AMX* vm, cell* parameters) //5C2500.
	{
		if (!VerifyScriptParameters(vm, parameters, 5, "AddFloatingDamageText")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "AddFloatingDamageText() is unable to find the requested entity.");
			return 0;
		}
		char text[128];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[2], text, sizeof(text)))
		{
			LogScriptError(vm, "AddFloatingDamageText() was unable to retrieve damage text.");
			return 0;
		}
		DWORD red = DWORD(parameters[3]), green = DWORD(parameters[4]), blue = DWORD(parameters[5]);
		DWORD id = entity->entityID;
		static_cast<JE::Server*>(g_pNetworkInstance->server)->FloatingDamageText(id,text,red,green,blue);
		return 1;
	}
}
