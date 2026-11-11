#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <vector>
#include <stdexcept>

namespace F3
{
	cell AMX_NATIVE_CALL NativeTeleportParty(AMX* vm,cell* parameters) //5CA3A0.
	{
		if (!VerifyScriptParameters(vm,parameters,2,"TeleportParty")) return 0;
		char mapName[260];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[1],mapName,sizeof(mapName)))
		{
			LogScriptError(vm,"TeleportParty() is unable to retrieve map name.");
			return 0;
		}
		//Original55C8D0 copies the entire vector before any action/map callbacks.
		//Duplicates and original member ordering survive the transfer loop.
		std::vector<JE::Entity*> members(JE::GetWorldInstance()->GetParty()->members);
		LONG formationIndex=0;
		for (std::vector<JE::Entity*>::iterator i=members.begin();i!=members.end();++i)
		{
			JE::Entity* entity=*i;
			entity->ClearActions();
			entity->QueueAction(JE::GetWorldInstance()->NewAction(1)); //Reload World after cancellation callbacks.
			//5CA47F reloads just the low BYTE of the native parameter each iteration.
			static_cast<JE::Server*>(g_pNetworkInstance->server)->MoveEntityToNamedMapEntry(entity,mapName,
				BYTE(parameters[2]),5-formationIndex,1);
			formationIndex=(formationIndex+1)%6;
		}
		return 1; //Explicit EAX1 at5CA4C7, also for an empty party.
	}
}
