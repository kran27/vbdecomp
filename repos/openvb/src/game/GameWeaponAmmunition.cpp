#include <winsock2.h>
#include "GameWeapon.hpp"
#include "GameAmmo.hpp"
#include "GameInventory.hpp"
#include "startup/GameServer.hpp"
#include "entity/EntityEnvironment.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <new>
#include <stdexcept>

namespace F3
{
	namespace
	{
		GameServer& AmmunitionServer()
		{
			GameServer* server=dynamic_cast<GameServer*>(JE::GetEntityEnvironment().GetServer());
			if (!server) throw std::logic_error("GameWeapon412B58: original unchecked NULL GameServer ammunition publisher");
			return *server;
		}
		void DeductStack(JE::Entity* stack, DWORD amount) //549D30, count never becomes0.
		{ if (stack->count956>amount) stack->count956-=amount; else stack->count956=1; }
	}

	void GameWeapon::DeductPrimaryAmmo(DWORD amount)
	{
		if (amount>primaryAmmoLoaded)
			JE::LogDebugString("GameWeapon::DeductPrimaryAmmo() - Attempt to deduct more ammo than the weapon had.\n");
		primaryAmmoLoaded-=amount;
		if (!primaryAmmoLoaded) primaryAmmo=NULL;
		GameServer& server=AmmunitionServer();
		server.SendPrimaryAmmo(entityID,primaryModifier,primaryAmmoLoaded);
	}
	void GameWeapon::DeductSecondaryAmmo(DWORD amount)
	{
		if (amount>secondaryAmmoLoaded)
			JE::LogDebugString("GameWeapon::DeductSecondaryAmmo() - Attempt to deduct more ammo than the weapon had.\n");
		secondaryAmmoLoaded-=amount;
		if (!secondaryAmmoLoaded) secondaryAmmo=NULL;
		GameServer& server=AmmunitionServer();
		server.SendSecondaryAmmo(entityID,secondaryModifier,secondaryAmmoLoaded);
	}

	BYTE GameWeapon::UnloadPrimaryAmmo(JE::Entity* owner)
	{
		if (!primaryAmmoLoaded) { primaryAmmo=NULL; return 1; }
		GameServer& server=AmmunitionServer();
		if (!owner || !owner->inventory)
			throw std::logic_error("GameWeapon413619: original unchecked NULL owner/inventory during ammo unload");
		JE::Inventory* inventoryOwner=owner->inventory;
		GameAmmo* stack=NULL;
		std::list<JE::Entity*>::iterator i=inventoryOwner->items.begin();
		const std::list<JE::Entity*>::iterator end=inventoryOwner->items.end();
		for (; i!=end; ++i)
		{
			GameItem* item=dynamic_cast<GameItem*>(*i);
			if (!item || item->entityClass!=9) continue;
			GameAmmo* candidate=dynamic_cast<GameAmmo*>(item);
			if (!candidate)
				throw std::logic_error("GameWeapon413697: original class9 item failed unchecked GameAmmo cast");
			if (candidate->caliber==primaryCaliber && candidate->modifier==primaryModifier)
			{
				stack=candidate;
				break;
			}
		}
		if (!stack)
		{
			GameAmmo* archetype=ResolveAmmunition(primaryCaliber,primaryModifier);
			if (!archetype)
				JE::FatalError("GameWeapon::UnloadPrimaryAmmo() - Unable to look up ammo. Caliber %d, modifier %d",
					primaryCaliber,primaryModifier);
			stack=new GameAmmo;
			if (!stack) throw std::logic_error("GameWeapon413720: original unchecked NULL newly allocated GameAmmo");
			stack->CopyFrom(archetype);
			GameInventory* inventory=dynamic_cast<GameInventory*>(owner->inventory);
			if (!inventory) throw std::logic_error("GameWeapon41375D: original unchecked GameInventory RTTI result");
			inventory->Add(stack);
			server.SendNewAmmoStack(owner->entityID,stack->entityID,primaryCaliber,primaryModifier);
		}
		stack->SetCount(primaryAmmoLoaded); //549D60 overwrites the prior stack count; it does not add.
		primaryAmmoLoaded=0;
		primaryAmmo=NULL;
		server.SendPrimaryAmmo(entityID,primaryModifier,0);
		server.SendEntityCount(stack->entityID,stack->count956);
		return 1;
	}
	BYTE GameWeapon::UnloadSecondaryAmmo(JE::Entity* owner)
	{
		if (!secondaryAmmoLoaded) { secondaryAmmo=NULL; return 1; }
		GameServer& server=AmmunitionServer();
		if (!owner || !owner->inventory)
			throw std::logic_error("GameWeapon413869: original unchecked NULL owner/inventory during ammo unload");
		JE::Inventory* inventoryOwner=owner->inventory;
		GameAmmo* stack=NULL;
		std::list<JE::Entity*>::iterator i=inventoryOwner->items.begin();
		const std::list<JE::Entity*>::iterator end=inventoryOwner->items.end();
		for (; i!=end; ++i)
		{
			GameItem* item=dynamic_cast<GameItem*>(*i);
			if (!item || item->entityClass!=9) continue;
			GameAmmo* candidate=dynamic_cast<GameAmmo*>(item);
			if (!candidate)
				throw std::logic_error("GameWeapon4138E7: original class9 item failed unchecked GameAmmo cast");
			if (candidate->caliber==secondaryCaliber && candidate->modifier==secondaryModifier)
			{
				stack=candidate;
				break;
			}
		}
		if (!stack)
		{
			GameAmmo* archetype=ResolveAmmunition(secondaryCaliber,secondaryModifier);
			if (!archetype)
				JE::FatalError("GameWeapon::UnloadPrimaryAmmo() - Unable to look up ammo. Caliber %d, modifier %d",
					secondaryCaliber,secondaryModifier);
			stack=new GameAmmo;
			if (!stack) throw std::logic_error("GameWeapon413970: original unchecked NULL newly allocated GameAmmo");
			stack->CopyFrom(archetype);
			GameInventory* inventory=dynamic_cast<GameInventory*>(owner->inventory);
			if (!inventory) throw std::logic_error("GameWeapon4139AD: original unchecked GameInventory RTTI result");
			inventory->Add(stack);
			server.SendNewAmmoStack(owner->entityID,stack->entityID,secondaryCaliber,secondaryModifier);
		}
		stack->SetCount(secondaryAmmoLoaded);
		secondaryAmmoLoaded=0;
		secondaryAmmo=NULL;
		server.SendPrimaryAmmo(entityID,secondaryModifier,0); //413A1D really calls primary publisher46D700.
		server.SendEntityCount(stack->entityID,stack->count956);
		return 1;
	}
	BYTE GameWeapon::ReloadPrimaryAmmo(GameAmmo* stack, JE::Entity* owner)
	{
		if (!stack || !owner)
		{ JE::LogDebugString("GameWeapon::ReloadPrimaryAmmo() - NULL ammo stack pointer.\n"); return 0; }
		if (!primaryAmmoCapacity)
		{ JE::LogDebugString("GameWeapon::ReloadPrimaryAmmo() - This weapon doesn't even use ammo.\n"); return 0; }
		if (stack->caliber!=primaryCaliber)
		{ JE::LogDebugString("GameWeapon::ReloadPrimaryAmmo() - Wrong caliber of ammo passed for reload.\n"); return 0; }
		const bool change=stack->modifier!=primaryModifier && primaryAmmoLoaded!=0;
		DWORD amount=primaryAmmoCapacity-primaryAmmoLoaded;
		if (!change && !amount) return 0;
		if (!dynamic_cast<GameInventory*>(owner->inventory))
		{ JE::LogDebugString("GameWeapon::ReloadPrimaryAmmo() - Unable to retrieve owner's inventory.\n"); return 0; }
		if (change) { UnloadPrimaryAmmo(owner); amount=primaryAmmoCapacity; }
		if (stack->count956<amount) amount=stack->count956;
		primaryAmmoLoaded+=amount;
		primaryModifier=stack->modifier;
		primaryAmmo=ResolveAmmunition(stack->caliber,stack->modifier);
		if (!primaryAmmo)
			JE::FatalError("GameWeapon::ReloadPrimaryAmmo - Unable to look up ammo: caliber %d, modifier %d",stack->caliber,stack->modifier);
		GameServer& server=AmmunitionServer();
		server.SendPrimaryAmmo(entityID,primaryModifier,primaryAmmoLoaded);
		if (stack->count956==amount)
		{
			server.RemoveItemFromInventory(owner,stack);
			stack->Destroy();
		}
		else { DeductStack(stack,amount); server.SendEntityCount(stack->entityID,stack->count956); }
		return 1;
	}
	BYTE GameWeapon::ReloadSecondaryAmmo(GameAmmo* stack, JE::Entity* owner)
	{
		if (!stack || !owner)
		{ JE::LogDebugString("GameWeapon::ReloadSecondaryAmmo() - NULL ammo stack pointer.\n"); return 0; }
		if (!secondaryAmmoCapacity)
		{ JE::LogDebugString("GameWeapon::ReloadSecondaryAmmo() - This weapon doesn't even use a 2ndary ammo.\n"); return 0; }
		if (stack->caliber!=secondaryCaliber)
		{ JE::LogDebugString("GameWeapon::ReloadSecondaryAmmo() - Wrong caliber of ammo passed for reload.\n"); return 0; }
		//413D08/413D14 deliberately use the PRIMARY modifier and loaded count.
		const bool change=stack->modifier!=primaryModifier && primaryAmmoLoaded!=0;
		DWORD amount=secondaryAmmoCapacity-secondaryAmmoLoaded;
		if (!change && !amount) return 0;
		if (!dynamic_cast<GameInventory*>(owner->inventory))
		{ JE::LogDebugString("GameWeapon::ReloadSecondaryAmmo() - Unable to retrieve owner's inventory.\n"); return 0; }
		if (change) UnloadSecondaryAmmo(owner); //Original retains pre-unload amount.
		if (stack->count956<amount) amount=stack->count956;
		secondaryAmmoLoaded=amount; //413D8C assigns, rather than adding prior ammo.
		secondaryModifier=stack->modifier;
		secondaryAmmo=ResolveAmmunition(stack->caliber,stack->modifier);
		if (!secondaryAmmo)
			JE::FatalError("GameWeapon::ReloadSecondaryAmmo - Unable to look up ammo: caliber %d, modifier %d",stack->caliber,stack->modifier);
		GameServer& server=AmmunitionServer();
		server.SendSecondaryAmmo(entityID,secondaryModifier,secondaryAmmoLoaded);
		//The equal-count secondary branch returns without removing or deducting it.
		if (stack->count956!=amount) { DeductStack(stack,amount); server.SendEntityCount(stack->entityID,stack->count956); }
		return 1;
	}
}
