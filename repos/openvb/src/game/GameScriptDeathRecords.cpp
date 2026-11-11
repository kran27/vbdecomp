#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include "../startup/NetworkComponent.hpp"
#include <stdexcept>
#include <string>

namespace F3
{
	cell AMX_NATIVE_CALL NativeRecordDeath(AMX* vm, cell* parameters) //5C85C0.
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "RecordDeath")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "RecordDeath() is unable to find the source entity.");
			return 0;
		}
		{
			std::string key("_DEAD_");
			key.append(entity->displayName);
			JE::IWorld* world = JE::g_World;
			DWORD count = DWORD(world->GetGlobalParameters()->GetInteger(std::string(key), "")) + 1;
			//The original reloads the published Network/Server after the read.
			static_cast<JE::Server*>(g_pNetworkInstance->server)->SetGlobal(std::string(key), LONG(count));
		}
		return 1;
	}

	cell AMX_NATIVE_CALL NativeHasDied(AMX* vm, cell* parameters) //5C8750.
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "HasDied")) return 0;
		char key[260] = "_DEAD_";
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], key + 6, 254))
		{
			LogScriptError(vm, "HasDied() is unable to retrieve the instance name.");
			return 0;
		}
		JE::IWorld* world = JE::g_World;
		return cell(world->GetGlobalParameters()->GetInteger(key, ""));
	}
}
