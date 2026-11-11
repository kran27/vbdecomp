#include <winsock2.h>
#include "network/EngineClientBinding.hpp"
#include "GameEquipmentActions.hpp"
#include "GameEquipmentPackets.hpp"
#include "GameEntity.hpp"
#include "GameInventory.hpp"
#include "GameItem.hpp"
#include "GameWeapon.hpp"
#include "entity/EntityEnvironment.hpp"
#include "model/ModelInstance.hpp"
#include "startup/GameServer.hpp"
#include "startup/GameClient.hpp"
#include "game/ActionbarInterface.hpp"
#include "startup/NetworkComponent.hpp"
#include <stdexcept>

namespace F3
{
	namespace
	{
		GameServer* EquipmentServer()
		{
			GameServer* server=dynamic_cast<GameServer*>(JE::GetEntityEnvironment().GetServer());
			if (!server) throw std::logic_error("Game equipment action: original GameServer owner required");
			return server;
		}
	}

	GameActionEquip::GameActionEquip() : recoveryEnd(0), itemID(0), slot(0), value28(0), flag2C(0), equipped(0) {}
	int GameActionEquip::Start(DWORD time) //4050A0.
	{
		JE::ActionEquip::Start(time);
		GameEntity* entity=dynamic_cast<GameEntity*>(owner);
		LONG duration=0;
		if (entity)
		{
			GameWeapon* weapon=dynamic_cast<GameWeapon*>(JE::GetEntityPtr(itemID));
			switch (slot)
			{
			case 2: case 3: case 4: case 5:
				if (entity->GetActiveWeaponSlot()==slot)
				{
					entity->SetWeaponStance(weapon);
					duration=entity->QueueAnimationEvent(21,0);
				}
				break;
			}
		}
		recoveryEnd=time+DWORD(duration);
		return 0;
	}
	void GameActionEquip::OnEquipCue() //405160.
	{
		if (equipped) return;
		GameItem* item=dynamic_cast<GameItem*>(JE::GetEntityPtr(itemID));
		if (item) EquipmentServer()->EquipItem(owner,item,slot,1);
		equipped=1;
	}
	void GameActionEquip::Cancel() { OnEquipCue(); JE::ActionEquip::Cancel(); }
	int GameActionEquip::Update(DWORD time, DWORD budget) //4051F0.
	{
		if (!itemID) { completed=1; return 0; }
		if (recoveryEnd && time>=recoveryEnd) { OnEquipCue(); completed=1; }
		return JE::ActionEquip::Update(time,budget);
	}

	GameActionUnequip::GameActionUnequip() : recoveryEnd(0), itemID(0), unequipped(0) {}
	int GameActionUnequip::Start(DWORD time) //407B20.
	{
		JE::ActionUnequip::Start(time);
		GameEntity* entity=dynamic_cast<GameEntity*>(owner);
		LONG duration=0;
		if (entity)
		{
			GameItem* item=dynamic_cast<GameItem*>(JE::GetEntityPtr(itemID));
			if (item && item->model && item->model->IsAttached())
				duration=entity->QueueAnimationEvent(22,0);
		}
		recoveryEnd=time+DWORD(duration);
		return 0;
	}
	void GameActionUnequip::OnUnequipCue() //407BC0.
	{
		if (unequipped) return;
		GameItem* item=dynamic_cast<GameItem*>(JE::GetEntityPtr(itemID));
		if (item) EquipmentServer()->UnequipItem(owner,item);
		unequipped=1;
	}
	void GameActionUnequip::Cancel() { OnUnequipCue(); JE::ActionUnequip::Cancel(); }
	int GameActionUnequip::Update(DWORD time, DWORD budget) //407C50.
	{
		if (!itemID) { completed=1; return 0; }
		if (recoveryEnd && time>=recoveryEnd) { OnUnequipCue(); completed=1; }
		return JE::ActionUnequip::Update(time,budget);
	}

	GameActionSetActiveWeapon::GameActionSetActiveWeapon() : previousSlot(0), nextSlot(0), phase(0),
		nextItemID(0), previousItemID(0), waitingUnequip(0), unequipEnd(0), waitingEquip(0),
		equipEnd(0), equipped(0), unequipped(0) {}
	int GameActionSetActiveWeapon::Start(DWORD time) //407620.
	{
		GameInventory* inventory=dynamic_cast<GameInventory*>(owner->inventory);
		if (inventory)
		{
			GameItem* next=inventory->GetItem(nextSlot);
			GameItem* previous=inventory->GetItem(previousSlot);
			if (next) nextItemID=next->entityID;
			if (previous) previousItemID=previous->entityID;
			GameEntity* entity=dynamic_cast<GameEntity*>(owner);
			if (entity)
			{
				const DWORD selectedSlot=nextSlot;
				if (entity->GetActiveWeaponSlot()!=selectedSlot)
				{
					entity->SetActiveWeaponSlot(selectedSlot);
					static_cast<GameServer*>(g_pNetworkInstance->server)->SendActiveWeapon(entity->entityID,nextSlot);
					return JE::ActionSetActiveWeapon::Start(time);
				}
			}
		}
		completed=1;
		return JE::ActionSetActiveWeapon::Start(time);
	}
	void GameActionSetActiveWeapon::OnEquipCue() //407710.
	{
		if (equipped) return;
		equipped=1; //The original commits this guard before either RTTI lookup.
		GameEntity* entity=dynamic_cast<GameEntity*>(owner);
		if (!entity) return;
		GameItem* item=dynamic_cast<GameItem*>(JE::GetEntityPtr(nextItemID));
		if (!item) return;
		GameInventory* inventory=static_cast<GameInventory*>(entity->inventory);
		if (JE::GetEntityEnvironment().NetworkMode()&1) inventory->ApplyEquipEffects(item);
		inventory->ApplyItemAppearance(item,30,nextSlot);
	}
	void GameActionSetActiveWeapon::OnUnequipCue() //4077B0.
	{
		if (unequipped) return;
		unequipped=1;
		GameEntity* entity=dynamic_cast<GameEntity*>(owner);
		if (!entity) return;
		GameItem* item=dynamic_cast<GameItem*>(JE::GetEntityPtr(previousItemID));
		if (!item) return;
		GameInventory* inventory=static_cast<GameInventory*>(entity->inventory);
		if (JE::GetEntityEnvironment().NetworkMode()&1) inventory->RemoveEquipEffects(item);
		inventory->RemoveItemAppearance(item,previousSlot);
	}
	void GameActionSetActiveWeapon::Cancel() //407850.
	{
		OnUnequipCue();
		OnEquipCue();
		JE::ActionSetActiveWeapon::Cancel();
		JE::Client* client=static_cast<JE::Client*>(g_pNetworkInstance->client);
		GameClient* gameClient=dynamic_cast<GameClient*>(client);
		if (!gameClient) return;
		ActionbarInterface* actionbar = gameClient->Actionbar();
		if (actionbar)
		{
			GameEntity* entity=dynamic_cast<GameEntity*>(owner);
			if (entity)
			{
				actionbar->SetWeaponActive(0,entity->GetActiveWeaponSlot()==2);
				actionbar->SetWeaponActive(1,entity->GetActiveWeaponSlot()==3);
			}
			return;
		}
		//The original skips a missing optional Actionbar. An explicit workspace
		//capture remains separate from that native path.
		if (JE::GetEngineClientBinding(*client).unconsumedInterfaceAction)
			JE::GetEngineClientBinding(*client).unconsumedInterfaceAction(*client,0x407890,JE::GetEngineClientBinding(*client).unconsumedInterfaceActionContext);
	}
	int GameActionSetActiveWeapon::Update(DWORD time, DWORD budget) //407910.
	{
		if (completed) return 0;
		GameEntity* entity=dynamic_cast<GameEntity*>(owner);
		if (!entity || !dynamic_cast<GameInventory*>(entity->inventory)) { completed=1; return 0; }
		LONG duration=0;
		for (;;)
		{
			switch (phase)
			{
			case 0:
				if (dynamic_cast<GameItem*>(JE::GetEntityPtr(previousItemID)))
				{
					duration+=entity->QueueAnimationEvent(22,0);
					waitingUnequip=1;
					unequipEnd=time+DWORD(duration);
					entity->SetWeaponStance(NULL);
					phase=1;
					continue;
				}
				if (dynamic_cast<GameItem*>(JE::GetEntityPtr(nextItemID)))
				{
					entity->SetWeaponStance(NULL);
					waitingUnequip=1;
					unequipEnd=time+DWORD(duration);
					phase=1;
					continue;
				}
				entity->SetWeaponStance(NULL);
				phase=1;
				continue;
			case 1:
				if (waitingUnequip)
				{
					if (time<unequipEnd) return JE::ActionSetActiveWeapon::Update(time,budget);
					OnUnequipCue();
				}
				phase=2;
				continue;
			case 2:
				entity->SetWeaponStance(NULL);
				if (dynamic_cast<GameItem*>(JE::GetEntityPtr(nextItemID)))
				{
					if (entity->GetActiveWeaponSlot()==nextSlot) duration+=entity->QueueAnimationEvent(21,0);
					waitingEquip=1;
					equipEnd=time+DWORD(duration);
				}
				phase=3;
				continue;
			case 3:
				if (waitingEquip)
				{
					if (time<equipEnd) return JE::ActionSetActiveWeapon::Update(time,budget);
					OnEquipCue();
				}
				phase=4;
				continue;
			default:
				completed=1;
				return JE::ActionSetActiveWeapon::Update(time,budget);
			}
		}
	}
}
