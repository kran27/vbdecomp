#include "GameCreature.hpp"
#include "GameInventory.hpp"
#include "GameWeapon.hpp"
#include "scripting/ScriptSmall.hpp"
#include <stdexcept>

namespace F3
{
	BYTE GameCreature::ResolveWeaponAndAttackMode(GameWeapon** weapon, AttackModeIterator* mode, BYTE* unarmed)
	{
		//40B3A1 captures Client+8C before the active-slot/inventory children.
		void* debugWindow=JE::ScriptServices::GetDebugWindow();
		DWORD slot=GetActiveWeaponSlot();
		GameInventory* items=dynamic_cast<GameInventory*>(inventory);
		if (!items) return 0;
		GameItem* item=items->GetItem(slot);
		*unarmed=0;
		if (item)
		{
			if (item->entityClass!=6)
			{
				if (debugWindow)
					JE::ScriptServices::DebugOutput(debugWindow,
						"GameEntity::ResolveWeaponAndAttackMode() - Attack was attempted with non-weapon in hand.",
						128,128,128);
				return 0;
			}
			GameWeapon* selected=dynamic_cast<GameWeapon*>(item);
			*weapon=selected; //Original publishes the RTTI result before reading its mode.
			if (!selected)
				throw std::logic_error("ResolveWeaponAndAttackMode40B46F: original unchecked NULL GameWeapon owner");
			*mode=selected->currentAttackMode;
		}
		else
		{
			*unarmed=1;
			*mode=currentAttackMode;
			//Original deliberately leaves the prior weapon output untouched here.
		}
		//The output retains the original iterator; operator-> skips the list links.
		if ((*mode)->kind==3 && !*unarmed)
		{
			*unarmed=1;
			*mode=currentAttackMode;
		}
		return 1;
	}
}
