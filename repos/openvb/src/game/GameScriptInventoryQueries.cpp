#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "entity/Inventory.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <string>
#include <vector>

namespace F3
{
	cell AMX_NATIVE_CALL NativeHasItem(AMX* vm, cell* parameters) //5C84C0.
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "HasItem")) return 0;
		JE::Entity* entity = NULL;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "HasItem() was unable to find the requested entity.");
			return 0;
		}
		char name[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[2], name, sizeof(name)))
		{
			LogScriptError(vm, "HasItem() was unable to retrieve the item name.");
			return 0;
		}
		cell result;
		{
			const std::string& itemName = std::string(name);
			result = entity->inventory->CountResource(itemName);
		}
		return result;
	}

	cell AMX_NATIVE_CALL NativePartyHasItem(AMX* vm, cell* parameters) //5C88A0.
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "PartyHasItem")) return 0;
		char name[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[1], name, sizeof(name)))
		{
			LogScriptError(vm, "HasItem() was unable to retrieve the item name.");
			return 0;
		}
		DWORD result = 0;
		{
			std::vector<JE::Entity*> members(JE::GetWorldInstance()->GetParty()->members);
			for (std::vector<JE::Entity*>::iterator member = members.begin();
				member != members.end(); ++member)
			{
				JE::Entity* memberEntity = *member;
				std::string itemName(name);
				result += memberEntity->inventory->CountResource(itemName);
			}
		}
		return cell(result);
	}
}
