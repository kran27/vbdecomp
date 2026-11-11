#include "GameScriptNatives.hpp"
#include "GameCreature.hpp"
#include "GameInventory.hpp"
#include "GameItem.hpp"
#include "GameUsableObject.hpp"
#include "GameDoor.hpp"
#include "GameContainer.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <cstring>

namespace F3
{
	cell AMX_NATIVE_CALL NativeEquipItem(AMX* vm,cell* parameters) //45A580.
	{
		if (!VerifyScriptParameters(vm,parameters,3,"EquipItem")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"EquipItem() was unable to find the attacking entity."); return 0;
		}
		char name[128];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[2],name,sizeof(name)))
		{
			LogScriptError(vm,"EquipItem() was unable to find the Item Name."); return 0;
		}
		const DWORD requestedSlot=DWORD(parameters[3]);
		if (!dynamic_cast<GameCreature*>(entity))
		{
			LogScriptError(vm,"EquipItem() - Entity is not a creature"); return 0;
		}
		if (!entity->inventory)
		{
			LogScriptError(vm,"EquipItem() - Inventory not Found"); return 0;
		}
		GameInventory* inventory=dynamic_cast<GameInventory*>(entity->inventory);
		if (!inventory)
		{
			LogScriptError(vm,"EquipItem() - Game Inventory not Found"); return 0;
		}
		JE::Entity* item;
		{ std::string resource(name); item=inventory->FindResource(resource); }
		return item && inventory->Equip(item,requestedSlot,1) ? 1 : 0;
	}
	cell AMX_NATIVE_CALL NativeIsItemEquipped(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 2, "IsItemEquipped")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "IsItemEquipped() was unable to find the entity.");
			return 0;
		}
		char name[128];
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[2], name, sizeof(name)))
		{
			LogScriptError(vm, "IsItemEquipped() was unable to find the Item Name.");
			return 0;
		}
		if (!dynamic_cast<GameCreature*>(entity))
		{
			LogScriptError(vm, "IsItemEquipped() - Entity is not a creature");
			return 0;
		}
		if (!entity->inventory)
		{
			LogScriptError(vm, "IsItemEquipped() - Inventory not Found");
			return 0;
		}
		GameInventory* inventory=dynamic_cast<GameInventory*>(entity->inventory);
		if (!inventory)
		{
			LogScriptError(vm, "IsItemEquipped() - Game Inventory not Found");
			return 0;
		}
		std::list<JE::Entity*> equippedItems, unequippedItems;
		{
			std::string resource(name);
			inventory->FindItemsByResource(resource, equippedItems, unequippedItems);
		} //The temporary name dies before the actual equipped-list count read.
		if (!equippedItems.empty()) return 1; //45B8AA tests the count before both cleanup exits.
		return 0;
	}

	cell AMX_NATIVE_CALL NativeGetItemType(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetItemType")) return 0;
		JE::Entity* entity=NULL;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "GetItemType() - Unable to resolve requested entity.");
			return 0;
		}
		GameItem* item=dynamic_cast<GameItem*>(entity);
		if (!item)
		{
			LogScriptError(vm, "GetItemType() - Entity is not an Item");
			return 0;
		}
		return cell(item->value160); //Original root+A0, distinct from Entity.entityClass.
	}

	cell AMX_NATIVE_CALL NativeGetUsableObjectType(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetUsableObjectType")) return 0;
		JE::Entity* entity=NULL;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "GetUsableObjectType() - Unable to resolve requested entity.");
			return 0;
		}
		GameUsableObject* usable=dynamic_cast<GameUsableObject*>(entity);
		if (!usable)
		{
			LogScriptError(vm, "GetUsableObjectType() - Entity is not an usable ubject");
			return 0;
		}
		return cell(usable->value160); //Original root+A0, including Door/Container inheritance.
	}

	cell AMX_NATIVE_CALL NativeIsLocked(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "IsLocked")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "IsLocked() was unable to find the entity.");
			return 0;
		}
		GameDoor* door=dynamic_cast<GameDoor*>(entity);
		if (!door)
		{
			LogScriptError(vm, "IsLocked() - Entity is not a door");
			return 0;
		}
		return door->locked; //Raw BYTE+BD, no Container branch in original.
	}

	cell AMX_NATIVE_CALL NativeGetLockDC(AMX* vm, cell* parameters)
	{
		if (!VerifyScriptParameters(vm, parameters, 1, "GetLockDC")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm, DWORD(parameters[1]), &entity, 1))
		{
			LogScriptError(vm, "GetLockDC() was unable to find the entity.");
			return 0;
		}
		GameContainer* container=dynamic_cast<GameContainer*>(entity);
		if (container) return cell(container->valueC0);
		GameDoor* door=dynamic_cast<GameDoor*>(entity);
		if (door) return cell(door->valueC0);
		LogScriptError(vm, "GetLockDC() entity passed in was neither a container nor a door.");
		return 0;
	}
}
