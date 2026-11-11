#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <stdexcept>

namespace F3
{
	cell AMX_NATIVE_CALL NativeGiveNewItem(AMX* vm,cell* parameters) //5C8A60.
	{
		if (!VerifyScriptParameters(vm,parameters,3,"GiveNewItem")) return 0;
		JE::Entity* owner;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&owner,1))
		{
			LogScriptError(vm,"GiveNewItem() was unable to find the entity.");
			return 0;
		}
		char resource[260];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[2],resource,sizeof(resource)))
		{
			LogScriptError(vm,"GiveNewItem() was unable to retrieve the item name.");
			return 0;
		}
		DWORD count=DWORD(parameters[3]);
		for (DWORD i=0;i<count;++i)
		{
			JE::Server* server=static_cast<JE::Server*>(g_pNetworkInstance->server);
			JE::Entity* item=server->CreateEntity(resource,0,0);
			if (!item)
			{
				LogScriptError(vm,"GiveNewItem() was unable to create an instance of item %s.",resource);
				return 0;
			}
			server=static_cast<JE::Server*>(g_pNetworkInstance->server); //Original reloads before each inventory call.
			server->AddItemToInventory(owner,item);
		}
		return 1; //Count0 succeeds; failure preserves already-created/transferred items.
	}
}
