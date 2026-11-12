#include <winsock2.h>
#include "GameActionReload.hpp"
#include "GameCreature.hpp"
#include "GameWeapon.hpp"
#include "GameAmmo.hpp"
#include "GameInventory.hpp"
#include "scripting/ScriptSmall.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <stdexcept>

namespace F3
{
	GameActionReload::GameActionReload() : creature(NULL), weapon(NULL), weaponID(0),
		stack(NULL), stackID(0), secondary(0), completed(0), flag2E(0), cost(6),
		firstUpdate(1), animationEnd(0)
	{ kind=1; } //Owner/minimumBudget remain original-constructor-unwritten.
	BYTE GameActionReload::IsComplete() const { return completed; }
	void GameActionReload::Cancel() {}
	int GameActionReload::Finish(DWORD) { return 0; }
	int GameActionReload::Start(DWORD)
	{
		if (weapon) weaponID=weapon->entityID;
		if (stack) stackID=stack->entityID;
		return 0;
	}
	DWORD GameActionReload::PlayReloadAnimation()
	{
		DWORD duration=creature->QueueAnimationEvent(120,0);
		return duration ? duration : 1;
	}
	int GameActionReload::Update(DWORD time, DWORD budget)
	{
		DWORD spent=0;
		creature=dynamic_cast<GameCreature*>(owner);
		if (!creature)
			JE::FatalError("GameActionReload::OnServerUpdate() - This action doesn't belong to a game creature.");
		if (weaponID)
		{
			weapon=dynamic_cast<GameWeapon*>(JE::GetEntityPtr(weaponID));
			if (!weapon)
			{
				JE::LogDebugString("GameActionReload::OnServerUpdate() - lost weapon between updates.\n");
				completed=1; return 0;
			}
		}
		if (stackID)
		{
			stack=dynamic_cast<GameAmmo*>(JE::GetEntityPtr(stackID));
			if (!stack)
			{
				JE::LogDebugString("GameActionReload::OnServerUpdate() - lost ammo between updates.\n");
				completed=1; return 0;
			}
		}
		if (!weapon)
		{
			DWORD slot=creature->GetActiveWeaponSlot(); //463B60.
			GameInventory* inventory=dynamic_cast<GameInventory*>(owner->inventory);
			if (!inventory)
				JE::FatalError("GameActionReload::OnServerCreate() - Unable to look up inventory.");
			GameItem* item=inventory->GetItem(slot);
			if (!item)
			{
				JE::LogDebugString("GameActionReload::OnServerCreate() - No weapon specified, and none equipped.");
				completed=1; return 0;
			}
			weapon=dynamic_cast<GameWeapon*>(item);
			if (!weapon)
			{
				JE::LogDebugString("GameActionReload::OnServerCreate() - The equipped item is not a weapon.");
				completed=1; return 0;
			}
			weaponID=weapon->entityID;
		}
		if (!stack)
		{
			const DWORD caliber=secondary ? weapon->secondaryCaliber : weapon->primaryCaliber;
			const DWORD modifier=secondary ? weapon->secondaryModifier : weapon->primaryModifier;
			for (std::list<JE::Entity*>::iterator i=owner->inventory->items.begin();
				i!=owner->inventory->items.end(); ++i)
			{
				GameItem* item=dynamic_cast<GameItem*>(*i);
				if (!item || item->entityClass!=9) continue;
				GameAmmo* ammo=dynamic_cast<GameAmmo*>(item);
				if (ammo->caliber==caliber && ammo->modifier==modifier) { stack=ammo; break; }
			}
			if (!stack) { completed=1; return 0; }
			//Original auto-selection does not publish stackID here.
		}
		if (firstUpdate==1)
		{
			DWORD duration=secondary ? weapon->secondaryReloadDuration : weapon->primaryReloadDuration;
			if (duration) cost=duration;
			firstUpdate=0;
		}
		else if (animationEnd)
		{
			if (animationEnd<=time)
			{
				void* debug=JE::ScriptServices::GetDebugWindow();
				if (debug) JE::ScriptServices::DebugOutput(debug,"Calling weapon's Reload function now.",0,64,64);
				if (secondary) weapon->ReloadSecondaryAmmo(stack,owner);
				else weapon->ReloadPrimaryAmmo(stack,owner);
				completed=1; //Actual action ignores the weapon method's result.
			}
		}
		else if (budget>=cost)
		{
			spent=cost;
			animationEnd=PlayReloadAnimation(); //407532 does not add current time.
		}
		return static_cast<int>(spent);
	}
}
