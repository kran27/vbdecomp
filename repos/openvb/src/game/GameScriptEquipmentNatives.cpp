#include "GameScriptNatives.hpp"
#include "GameScriptGameEntityContext.hpp"
#include "GameCreature.hpp"
#include "GameInventory.hpp"
#include "GameItem.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <cstring>

namespace F3
{

	cell AMX_NATIVE_CALL NativeUnequipItem(AMX* vm,cell* parameters) //45A6F0.
	{
		//45A712 reuses the EquipItem label for the parameter-count diagnostic.
		if (!VerifyScriptParameters(vm,parameters,3,"EquipItem")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"UnequipItem() was unable to find the attacking entity.");
			return 0;
		}
		char name[128];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[2],name,sizeof(name)))
		{
			LogScriptError(vm,"UnequipItem() was unable to find the Item Name.");
			return 0;
		}
		DWORD slot=DWORD(parameters[3]);
		if (!dynamic_cast<GameCreature*>(entity))
		{
			LogScriptError(vm,"UnequipItem() - Entity is not a creature");
			return 0;
		}
		if (!entity->inventory)
		{
			LogScriptError(vm,"UnequipItem() - Inventory not Found");
			return 0;
		}
		GameInventory* inventory=dynamic_cast<GameInventory*>(entity->inventory);
		if (!inventory)
		{
			LogScriptError(vm,"UnequipItem() - Game Inventory not Found");
			return 0;
		}
		if (slot==7)
		{
			std::list<JE::Entity*> equippedItems,unequippedItems;
			{
				std::string resource(name);
				inventory->FindItemsByResource(resource,equippedItems,unequippedItems);
			} //45A85D destroys the temporary string before callbacks.
			cell result=0;
			std::list<JE::Entity*>::iterator i;
			for (i=equippedItems.begin();i!=equippedItems.end();++i)
			{
				result=1;
				inventory->Unequip(*i);
			}
			return result;
		}
		GameItem* item=inventory->GetItem(slot); //Original has no native slot clamp.
		if (item)
		{
			if (!std::strcmp(item->resourceName.c_str(),name))
			{
				inventory->Unequip(item);
				return 1;
			}
			if (!std::strcmp(name,"*")) inventory->Unequip(item);
		}
		//45A93F..45A942: wildcard removal still returns0 after the callback.
		return 0;
	}

	cell AMX_NATIVE_CALL NativeGetWeaponCombo(AMX* vm,cell* parameters) //45A9F0.
	{
		GameEntity* entity;
		if (VerifyScriptParameters(vm,parameters,1,"GetWeaponCombo")
			&& !ResolveGameEntity(vm,DWORD(parameters[1]),&entity))
			LogScriptError(vm,"GetWeaponCombo() was unable to find the requested entity.");
		return 0; //Original45AA35, including a successfully resolved owner.
	}

	cell AMX_NATIVE_CALL NativeSetWeaponCombo(AMX* vm,cell* parameters) //45AA40.
	{
		GameEntity* entity;
		if (VerifyScriptParameters(vm,parameters,2,"SetWeaponCombo")
			&& !ResolveGameEntity(vm,DWORD(parameters[1]),&entity))
			LogScriptError(vm,"SetWeaponCombo() was unable to find the requested entity.");
		return 0; //Original45AA85; second argument is not consumed by this prototype.
	}
}
