#include "GameScriptNatives.hpp"
#include "GameCreature.hpp"
#include "GameInventory.hpp"
#include "GameWeapon.hpp"
#include <stdexcept>

namespace F3
{
	cell AMX_NATIVE_CALL NativeGetWeaponRange(AMX* vm, cell* parameters) //45B620.
	{
		if (!VerifyScriptParameters(vm,parameters,1,"GetWeaponRange")) return 0;
		JE::Entity* entity=NULL;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"GetWeaponRange() - Unable to resolve requested entity.");
			return 0;
		}
		GameCreature* creature=dynamic_cast<GameCreature*>(entity);
		if (!creature)
		{
			LogScriptError(vm,"GetWeaponRange() - Entity is not a creature");
			return 0;
		}
		if (!entity->inventory)
		{
			LogScriptError(vm,"GetWeaponRange() - Inventory not Found");
			return 0;
		}
		GameInventory* inventory=dynamic_cast<GameInventory*>(entity->inventory);
		if (!inventory)
		{
			LogScriptError(vm,"GetWeaponRange() - Game Inventory not Found");
			return 0;
		}
		GameItem* item=inventory->GetItem(creature->GetActiveWeaponSlot());
		AttackModeIterator mode;
		if (item)
		{
			GameWeapon* weapon=dynamic_cast<GameWeapon*>(item);
			mode=weapon->currentAttackMode;
		}
		else
		{
			mode=creature->currentAttackMode;
		}
		return mode == AttackModeIterator() ? 0 : cell(mode->range); //Original node+2C, nullable real iterator.
	}

	cell AMX_NATIVE_CALL NativeNullQuest(AMX*, cell*) //420990, shared by all four actual entries.
	{
		return 1; //Original MOV EAX,1 / RET; no argument checks or quest mutation.
	}
}
