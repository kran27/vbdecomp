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
	cell AMX_NATIVE_CALL NativeTransferItem(AMX* vm,cell* parameters) //5C9780.
	{
		if (!VerifyScriptParameters(vm,parameters,4,"TransferItem")) return 0;
		JE::Entity* source;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&source,1))
		{
			LogScriptError(vm,"TransferItem() was unable to find the source entity.");
			return 0;
		}
		JE::Entity* target;
		if (!ResolveScriptEntity(vm,DWORD(parameters[2]),&target,1))
		{
			LogScriptError(vm,"TransferItem() was unable to find the target entity.");
			return 0;
		}
		char name[260];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[3],name,sizeof(name)))
		{
			LogScriptError(vm,"TransferItem() was unable to retrieve the item name.");
			return 0;
		}
		DWORD count=DWORD(parameters[4]);
		for (DWORD i=0;i<count;++i)
		{
			JE::Entity* item;
			{
				std::string resource(name);
				item=source->inventory->FindResource(resource);
			} //Original destroys this temporary before the server callback.
			if (!item)
			{
				LogScriptError(vm,"TransferItem() was unable to find the requested item in the owner's inventory.");
				return 0; //Earlier transfers remain published.
			}
			static_cast<JE::Server*>(g_pNetworkInstance->server)->AddItemToInventory(target,item);
		}
		return 1; //5C98E9 explicitly writes EAX1, including count0.
	}

	cell AMX_NATIVE_CALL NativeDropItem(AMX* vm,cell* parameters) //5C9930.
	{
		if (!VerifyScriptParameters(vm,parameters,2,"DropItem")) return 0;
		JE::Entity* source;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&source,1))
		{
			LogScriptError(vm,"DropItem() was unable to find the source entity.");
			return 0;
		}
		char name[260];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[2],name,sizeof(name)))
		{
			LogScriptError(vm,"DropItem() was unable to retrieve the item name.");
			return 0;
		}
		JE::Entity* item;
		{
			std::string resource(name);
			item=source->inventory->FindResource(resource);
		}
		if (!item)
		{
			LogScriptError(vm,"DropItem() was unable to find the requested item in the owner's inventory.");
			return 0;
		}
		DWORD map=source->mapHandle;
		D3DXVECTOR3 position;
		position.x=source->position.x;
		position.y=source->position.y;
		position.z=source->position.z;
		D3DXVECTOR3 rotation;
		rotation.x=source->rotation.x;
		rotation.y=source->rotation.y;
		rotation.z=source->rotation.z;
		static_cast<JE::Server*>(g_pNetworkInstance->server)->RemoveItemFromInventory(source,item);
		static_cast<JE::Server*>(g_pNetworkInstance->server)->MoveEntityToMap(item,map,position,rotation,0);
		return 1; //5C9A84, confirmed despite the retained void annotation.
	}
}
