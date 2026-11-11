#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "GameActionInitiateDialogue.hpp"
#include "entity/Entity.hpp"
#include "../startup/NetworkComponent.hpp"
#include "network/EngineServer.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeStartDialogue(AMX* vm, cell* parameters) //45A040.
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "StartDialogue")) return 0;
		JE::Entity* source;
		if (!ResolveScriptEntity(vm, parameters[1], &source, 1))
		{
			LogScriptError(vm, "StartDialogue() was unable to find the source entity.");
			return 0;
		}
		JE::Entity* target;
		if (!ResolveScriptEntity(vm, parameters[2], &target, 1))
		{
			LogScriptError(vm, "StartDialogue() was unable to find the target entity.");
			return 0;
		}
		source->ClearActions();
		GameActionInitiateDialogue* action = new GameActionInitiateDialogue;
		action->target = target;
		action->useTargetDialogue = 0;
		source->QueueAction(action);
		return 1;
	}
	cell AMX_NATIVE_CALL NativeIsInDialogue(AMX* vm, cell* parameters) //5C39D0.
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "IsInDialogue")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, parameters[1], &entity, 1))
		{
			LogScriptError(vm, "IsUnderFOW() is unable to find the source entity.");
			return 0;
		}
		DWORD id = entity->entityID;
		JE::Server* server = static_cast<JE::Server*>(g_pNetworkInstance->server);
		return server->dialogue->IsInDialogue(id);
	}
}
