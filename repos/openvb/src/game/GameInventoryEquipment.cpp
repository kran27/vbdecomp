#include <winsock2.h>
#include "startup/NetworkComponent.hpp"
#include "GameInventory.hpp"
#include "GameItem.hpp"
#include "GameWeapon.hpp"
#include "GameCreature.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "entity/Effect.hpp"
#include "model/CommonException.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <stdexcept>

namespace F3
{
	GameInventory::~GameInventory()
	{
		for (std::list<JE::Entity*>::iterator i = items.begin(); i != items.end(); ++i) OnRemove(*i);
		//Base560020 unlinks each item and schedules actual deferred Destroy.
	}

	BYTE GameInventory::IsPartyOwner()
	{
		if (JE::g_EntityEnvironment)
		{
			JE::Entity* owner = JE::GetEntityPtr(ownerEntityID);
			return JE::GetEntityEnvironment().IsPartyMember(owner);
		}
		JE::IWorld* world = JE::g_World;
		JE::Party* party = world->GetParty();
		return party->Contains(JE::GetEntityPtr(ownerEntityID));
	}
	BYTE GameInventory::PartyContains(JE::Entity* entity)
	{
		//469298 queries the display-name string at Entity+1C0; the value is irrelevant.
		JE::IWorld* world = JE::g_World;
		JE::ItemRegistry* registry = world->GetItemRegistry();
		return registry->find(entity->displayName) != registry->end();
	}
	void GameInventory::AddPartyItem(GameItem* item)
	{
		JE::IWorld* world = JE::g_World;
		JE::ItemRegistry* registry = world->GetItemRegistry();
		(*registry)[item->resourceName] = 0; //Actual5A04B0 assigns, not increments.
		delete static_cast<JE::Entity*>(item); //469159 calls the actual virtual deleting destructor.
	}
	void GameInventory::RemovePartyItem(JE::Entity* entity)
	{
		JE::IWorld* world = JE::g_World;
		JE::ItemRegistry* registry = world->GetItemRegistry();
		registry->erase(entity->displayName); //Original key differs from Add's resourceName.
	}
	DWORD GameInventory::NetworkMode()
	{
		return JE::GetEntityEnvironment().NetworkMode();
	}
	void GameInventory::ApplyEquipEffects(GameItem* item)
	{
		std::list<JE::Effect*>* prototypes=item->GetEffects(0);
		if (prototypes->empty()) return;
		for (std::list<JE::Effect*>::iterator i=prototypes->begin();i!=prototypes->end();++i)
		{
			JE::Effect* prototype=*i;
			if (!(g_pNetworkInstance->GetMode()&1))
				throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
			JE::IWorldServer* world=JE::g_World;
			JE::Effect* effect=world->NewEffect(prototype->type);
			effect->CopyFrom(*prototype);
			effect->SetCreator(item->entityID);
			JE::Entity* owner=JE::GetEntityPtr(ownerEntityID);
			effect->SetTarget(1,item->entityID);
			effect->SetTarget(2,owner->entityID);
			effect->SetTarget(0,owner->entityID);
			effect->SetTarget(3,0);
			const DWORD effectID=effect->GetID(); //Capture before AddEffect may synchronously destroy it.
			DWORD targetID=0;
			switch (effect->GetRole())
			{
			case 0: case 2:
				targetID=owner->entityID;
				owner->AddEffect(effect);
				break;
			case 1:
				targetID=item->entityID;
				item->AddEffect(effect);
				break;
			case 3:
				JE::LogDebugString("ApplyEquipEffects() - An equip effect had target of WORLD.\n");
				effect->Destroy();
				continue;
			default:
				JE::FatalError("ApplyEquipEffects() - Unknown target type.");
				continue;
			}
			if (!targetID) continue;
			const DWORD itemID=item->entityID;
			std::map<DWORD,std::list<EquipmentEffectTarget> >::iterator entry=
				equippedEffects.find(itemID);
			if (entry==equippedEffects.end())
			{
				EquipmentEffectTarget value;
				value.entityID=targetID;
				value.effects.push_back(effectID);
				equippedEffects[item->entityID].push_back(value);
			}
			else
			{
				std::list<EquipmentEffectTarget>& targets=entry->second;
				std::list<EquipmentEffectTarget>::iterator target=targets.begin();
				for (;target!=targets.end();++target)
					if (target->entityID==targetID) break;
				if (target==targets.end())
				{
					EquipmentEffectTarget value;
					value.entityID=targetID;
					value.effects.push_back(effectID);
					equippedEffects[item->entityID].push_back(value);
				}
				else target->effects.push_back(effectID);
			}
		}
	}
	void GameInventory::RemoveEquipEffects(GameItem* item)
	{
		DWORD itemID = item->entityID;
		std::map<DWORD,std::list<EquipmentEffectTarget> >::iterator i = equippedEffects.find(itemID);
		if (i == equippedEffects.end()) return;
		while (!i->second.empty())
		{
			EquipmentEffectTarget& target = i->second.front();
			JE::Entity* entity = JE::GetEntityPtr(target.entityID);
			if (entity)
				for (std::list<DWORD>::iterator effect=target.effects.begin();effect!=target.effects.end();++effect)
					entity->CompleteEffect(*effect);
			i->second.pop_front(); // Missing targets still release their borrowed ID lists.
		}
		equippedEffects.erase(i);
	}

	void GameInventory::Add(JE::Entity* entity)
	{
		GameItem* item = dynamic_cast<GameItem*>(entity);
		if (item && item->value160 == 5)
		{
			JE::IWorld* world = JE::g_World;
			JE::Party* party = world->GetParty();
			if (party->Contains(JE::GetEntityPtr(ownerEntityID)) == 1)
			{
				JE::IWorld* registryWorld = JE::g_World;
				registryWorld->GetItemRegistry()->RegisterName(entity->resourceName);
				delete item;
				return;
			}
		}
		JE::Inventory::Add(entity);
		value44 += dynamic_cast<GameEntity*>(entity)->stringRefs[5];
		GameWeapon* weapon = dynamic_cast<GameWeapon*>(entity);
		if (weapon) weapon->RebuildAttackModes(JE::GetEntityPtr(ownerEntityID));
	}

	void GameInventory::Remove(JE::Entity* entity)
	{
		std::list<InventoryPendingRecord>::iterator i = pendingRecords.begin();
		while (i != pendingRecords.end())
		{
			if (i->values[2] == entity->entityID)
				//46959F discards the returned successor and retains the invalid iterator.
				pendingRecords.erase(i);
			else
				++i;
		}
		GameItem* item = dynamic_cast<GameItem*>(entity);
		if (item && item->value160 == 5)
		{
			JE::IWorld* world = JE::g_World;
			JE::Party* party = world->GetParty();
			if (party->Contains(JE::GetEntityPtr(ownerEntityID)) == 1)
			{
				JE::IWorld* registryWorld = JE::g_World;
				const std::string& key = entity->displayName;
				JE::ItemRegistry* registry = registryWorld->GetItemRegistry();
				registry->erase(key);
				return;
			}
		}
		Unequip(entity);
		JE::Inventory::Remove(entity);
		value44 -= dynamic_cast<GameEntity*>(entity)->stringRefs[5];
	}

	BYTE GameInventory::Contains(JE::Entity* entity)
	{
		JE::IWorld* world=JE::g_World;
		JE::Party* party=world->GetParty();
		if (party->Contains(JE::GetEntityPtr(ownerEntityID))==1)
		{
			JE::IWorld* registryWorld=JE::g_World;
			if (registryWorld->GetItemRegistry()->Contains(entity->displayName)==1) return 1;
		}
		return JE::Inventory::Contains(entity);
	}

	void GameInventory::OnRemove(JE::Entity* entity)
	{
		Unequip(entity);
		JE::Inventory::OnRemove(entity);
	}

	BYTE GameInventory::GetEquippedSlots(JE::Entity* entity, std::list<DWORD>& slots)
	{
		BYTE found = 0;
		for (DWORD i = 0; i < 6; ++i)
			if (equipped[i] == entity) { found = 1; slots.push_front(i); }
		return found;
	}

	BYTE GameInventory::SlotCompatible(DWORD slot, DWORD kind)
	{
		switch (slot)
		{
		case 0: if (kind == 0) return 1; break;
		case 1: if (kind == 1) return 1; break;
		case 2: case 3: case 4: case 5:
			if (kind == 2) return 1;
			break;
		}
		return 0;
	}
	DWORD GameInventory::DefaultSlot(DWORD kind)
	{
		DWORD result = 7;
		switch (kind)
		{
		case 0: result = 0; break;
		case 1: result = 1; break;
		}
		return result;
	}
	DWORD GameInventory::SelectEquipmentGroup(const std::list<DWORD>& slots)
	{
		DWORD group = 7;
		for (std::list<DWORD>::const_iterator i = slots.begin(); i != slots.end(); ++i)
		{
			switch (*i)
			{
			case 1: group = group != 0 ? 1 : 7; break;
			case 0: group = group != 1 ? 0 : 7; break;
			case 2: group = 2; break;
			case 3: group = 3; break;
			}
		}
		return group;
	}

	void GameInventory::SetItem(JE::Entity* entity, DWORD slot)
	{
		if (equipped[slot]) Unequip(equipped[slot]);
		if (entity && !Contains(entity)) Add(entity);
		equipped[slot] = entity;
	}
	void GameInventory::ClearSlot(DWORD slot) { equipped[slot] = NULL; }

	BYTE GameInventory::Equip(JE::Entity* entity, DWORD requestedSlot, BYTE notify)
	{
		if (!entity) return 0;
		std::list<DWORD> previous;
		if (GetEquippedSlots(entity, previous))
		{
			JE::LogDebugString("GameInventory::Equip() - Item that was already equipped trying to equip itself again.\n");
			return 0;
		}
		if (entity->inventoryOwnerID != ownerEntityID) Add(entity);
		GameItem* item = dynamic_cast<GameItem*>(entity);
		if (!item) return 0;
		std::list<DWORD> allowed(item->values168);
		std::list<DWORD>::const_iterator kind = allowed.begin();
		for (; kind != allowed.end(); ++kind)
			if (requestedSlot == 7 || SlotCompatible(requestedSlot, *kind)) break;
		if (kind == allowed.end()) return 0;
		std::list<DWORD> slots;
		DWORD bone = 0;
		for (kind = allowed.begin(); kind != allowed.end(); ++kind)
		{
			if (*kind == 2)
			{
				// Retain the original lookup/RTTI call even though this arm does not consume it.
				dynamic_cast<GameEntity*>(JE::GetEntityPtr(ownerEntityID));
				switch (requestedSlot)
				{
				case 2: case 7: slots.push_back(2); bone = 30; break;
				case 3: slots.push_back(3); bone = 30; break;
				}
			}
			else if (SlotCompatible(requestedSlot, *kind)) slots.push_back(requestedSlot);
			else
			{
				DWORD fallback = DefaultSlot(*kind);
				if (fallback == 7) return 0;
				slots.push_back(fallback);
			}
		}
		DWORD group = SelectEquipmentGroup(slots);
		for (std::list<DWORD>::const_iterator i = slots.begin(); i != slots.end(); ++i) SetItem(entity, *i);
		JE::Entity* owner = JE::GetEntityPtr(ownerEntityID);
		GameEntity* game = dynamic_cast<GameEntity*>(owner);
		if (!((group == 2 || group == 3) && game && game->GetActiveWeaponSlot() != group))
		{
			if ((g_pNetworkInstance->GetMode() & 1) && notify) ApplyEquipEffects(item);
			ApplyItemAppearance(item, bone, group);
			game = dynamic_cast<GameEntity*>(JE::GetEntityPtr(ownerEntityID));
			if (game) game->SetWeaponStance(NULL);
		}
		GameCreature* creature = dynamic_cast<GameCreature*>(owner);
		if (creature) creature->RecalculateArmor();
		return 1;
	}

	void GameInventory::Unequip(JE::Entity* entity)
	{
		if (!entity) return;
		std::list<DWORD> slots;
		if (!GetEquippedSlots(entity, slots)) return;
		DWORD group = SelectEquipmentGroup(slots);
		while (!slots.empty()) { ClearSlot(slots.front()); slots.pop_front(); }
		GameItem* item = dynamic_cast<GameItem*>(entity);
		JE::Entity* owner = JE::GetEntityPtr(ownerEntityID);
		GameEntity* game = dynamic_cast<GameEntity*>(owner);
		if (group == 2)
		{
			if (game && game->GetActiveWeaponSlot() != 2) goto refreshArmor;
		}
		else if (group == 3 && game && game->GetActiveWeaponSlot() != 3) goto refreshArmor;
		if (g_pNetworkInstance->GetMode() & 1) RemoveEquipEffects(item);
		RemoveItemAppearance(item, group);
	refreshArmor:
		GameCreature* creature = dynamic_cast<GameCreature*>(owner);
		if (creature)
		{
			creature->RecalculateArmor();
			creature->SetWeaponStance(NULL);
		}
	}
}
