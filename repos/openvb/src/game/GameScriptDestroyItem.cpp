#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "entity/Inventory.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <string>

namespace F3
{
	cell AMX_NATIVE_CALL NativeDestroyItem(AMX* vm, cell* parameters) //5C9650.
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "DestroyItem")) return 0;
		JE::Entity* owner;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &owner, 1))
		{
			LogScriptError(vm, "DestroyItem() is unable to find the requested entity.");
			return 0;
		}
		char name[260];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[2], name, sizeof(name)))
		{
			LogScriptError(vm, "DestroyItem() is unable to retrieve item name.");
			return 0;
		}
		JE::Entity* item;
		{
			std::string resource(name);
			item = owner->inventory->FindResource(resource);
		} //5C971C destroys the temporary before removal/equipment callbacks.
		if (!item)
		{
			LogScriptError(vm, "DestroyItem() is unable to find the item in the owner's inventory.");
			return 0;
		}
		static_cast<JE::Server*>(g_pNetworkInstance->server)->RemoveItemFromInventory(owner, item);
		item->Destroy(); //Deferred state2; original does not directly delete the item.
		return 1; //5C9755 explicitly writes EAX1, despite the old void annotation.
	}
}
