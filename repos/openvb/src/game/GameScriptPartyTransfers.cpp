#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/Inventory.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <string>
#include <vector>

namespace F3
{
	cell AMX_NATIVE_CALL NativeTransferItemFromParty(AMX* vm, cell* parameters) //5C9FF0.
	{
		if (!VerifyScriptParameters(vm, parameters, 3, "TransferItemFromParty")) return 0;
		JE::Entity* target;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &target, 1))
		{
			LogScriptError(vm, "TransferItemFromParty() was unable to find the target entity.");
			return 0;
		}
		char name[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[2], name, sizeof(name)))
		{
			LogScriptError(vm, "TransferItemFromParty() was unable to retrieve the item name.");
			return 0;
		}
		const DWORD count = DWORD(parameters[3]);
		const std::vector<JE::Entity*> members(JE::GetWorldInstance()->GetParty()->members); //55C8D0: copy before callbacks.
		for (DWORD transferred = 0; transferred < count; ++transferred)
		{
			JE::Entity* item = NULL;
			for (std::vector<JE::Entity*>::const_iterator member = members.begin(); member != members.end(); ++member)
			{
				JE::Entity* memberEntity = *member;
				{
					std::string resource(name);
					item = memberEntity->inventory->FindResource(resource);
				} //The temporary string is destroyed before the server callback.
				if (item) break;
			}
			if (!item)
			{
				LogScriptError(vm, "TransferItemFromParty() was unable to find the requested item in party's inventory.");
				return 0; //Earlier transfers remain published.
			}
			static_cast<JE::Server*>(g_pNetworkInstance->server)->AddItemToInventory(target, item);
		}
		return 1; //5CA1B1, including count0 with an empty party.
	}

	cell AMX_NATIVE_CALL NativeTransferItemToParty(AMX* vm, cell* parameters) //5CA1E0.
	{
		if (!VerifyScriptParameters(vm, parameters, 3, "TransferItemToParty")) return 0;
		JE::Entity* source;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &source, 1))
		{
			LogScriptError(vm, "TransferItemToParty() was unable to find the source entity.");
			return 0;
		}
		char name[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[2], name, sizeof(name)))
		{
			LogScriptError(vm, "TransferItemToParty() was unable to retrieve the item name.");
			return 0;
		}
		const std::vector<JE::Entity*> members(JE::GetWorldInstance()->GetParty()->members);
		const DWORD count = DWORD(parameters[3]);
		//5CA2AF dereferences the copied vector's first element before testing count.
		JE::Entity* target = members.front();
		for (DWORD transferred = 0; transferred < count; ++transferred)
		{
			JE::Entity* item;
			{
				std::string resource(name);
				item = source->inventory->FindResource(resource);
			}
			if (!item)
			{
				LogScriptError(vm, "TransferItemToParty() was unable to find the requested item in the owner's inventory.");
				return 0;
			}
			static_cast<JE::Server*>(g_pNetworkInstance->server)->AddItemToInventory(target, item);
		}
		return 1; //5CA356, after all transfers or count0 with a valid first member.
	}
}
