#include <cstdlib>
#include "GameServer.hpp"
#include "game/GameWorld.hpp"
#include "game/ClientUseRequests.hpp"
#include "game/ClientDoorRequests.hpp"
#include "game/ClientPickupRequests.hpp"
#include "game/GameActionGive.hpp"
#include "game/ClientAttackRequests.hpp"
#include "game/ClientDialogueActions.hpp"
#include "game/GameServerWeather.hpp"
#include "game/ClientAttackModeRequests.hpp"
#include "game/GameEquipmentActions.hpp"
#include "game/GameEntity.hpp"
#include "game/GameCreature.hpp"
#include "game/GameDoor.hpp"
#include "game/GameActionOperateDoor.hpp"
#include "game/GameActionOperateContainer.hpp"
#include "entity/Entity.hpp"
#include "entity/Inventory.hpp"
#include "entity/World.hpp"
#include "scripting/ScriptSmall.hpp"
#include <stdexcept>
#include <new>
#include "diagnostics/DebugOutput.hpp"

namespace F3
{
	void InitializeGameWorldAmmunition() //46CF70; incoming ECX is unused.
	{
		JE::IWorld* source=JE::g_World;
		GameWorld* world=dynamic_cast<GameWorld*>(source);
		if (world) world->InitializeAmmunition();
	}
	void GameServer::NotifyPerkRank(DWORD entityID, LONG perk, DWORD rank) //46D9E0.
	{
		JE::Net::Packet notification;
		notification.field0x30 = 94;
		notification.WriteDword(entityID);
		notification.WriteSignedDword(static_cast<JE::Net::Int32>(perk));
		notification.WriteDword(rank);
		Broadcast(notification, 1);
	}
	void GameServer::OnUserPacket(JE::Net::UInt32 clientIndex,JE::Net::Packet& packet)
	{
		if (ReceiveGameServerUseRequest(packet) || ReceiveGameServerOperateRequest(packet)
			|| ReceiveGameServerPickupRequest(packet) || ReceiveGameServerGiveRequest(packet)
			|| ReceiveGameServerAttackRequest(packet)
			|| ReceiveGameServerDialogueRequest(*this,packet)
			|| ReceiveGameServerWeatherRequest(*this,packet)
			|| ReceiveGameServerAttackModeRequest(*this,packet)
			|| ReceiveGameServerReloadRequest(packet)) return;
		switch (packet.field0x30)
		{
		case 11: //46DDCC..46DE2B, shared QueueAction tail46E8A0.
		{
			JE::Net::UInt32 entityID;
			packet.ReadDword(entityID);
			JE::Entity* base = JE::GetEntityPtr(entityID);
			if (!base) return;
			GameEntity* entity = dynamic_cast<GameEntity*>(base);
			if (!entity) return;
			entity->ClearActions();
			entity->QueueAction(JE::g_World->NewAction(0));
			return;
		}
		case 95:
		{
			JE::Net::UInt32 ownerID,itemID,sourceID,slot;
			packet.ReadDword(ownerID); packet.ReadDword(itemID);
			packet.ReadDword(sourceID); packet.ReadDword(slot);
			JE::Entity* owner=JE::GetEntityPtr(ownerID);
			JE::Entity* source=JE::GetEntityPtr(sourceID);
			JE::Entity* item=source ? source->inventory->FindByID(itemID) : NULL;
			//The original failure reply dereferences these failed pointers for
			//IDs. Retain that unsupported boundary rather than inventing a reply.
			if (!owner || !source || !item)
				throw std::logic_error("GameServer46E725: original invalid equipment request dereferences failed entity lookup");
			GameActionEquip* action=static_cast<GameActionEquip*>(JE::g_World->NewAction(9));
			action->itemID=item->entityID; action->slot=slot; action->value28=30; action->flag2C=1;
			owner->QueueAction(action);
			return;
		}
		case 96:
		{
			JE::Net::UInt32 ownerID,itemID;
			packet.ReadDword(ownerID); packet.ReadDword(itemID);
			JE::Entity* owner=JE::GetEntityPtr(ownerID);
			JE::Entity* item=owner ? owner->inventory->FindByID(itemID) : NULL;
			if (!owner || !item)
				throw std::logic_error("GameServer46E7E0: original invalid unequip request dereferences failed entity lookup");
			GameActionUnequip* action=static_cast<GameActionUnequip*>(JE::g_World->NewAction(10));
			action->itemID=item->entityID;
			owner->QueueAction(action);
			return;
		}
		case 89:
		{
			JE::Net::UInt32 ownerID,slot;
			packet.ReadDword(ownerID); packet.ReadDword(slot);
			GameEntity* owner=dynamic_cast<GameEntity*>(JE::GetEntityPtr(ownerID));
			if (!owner) return;
			GameActionSetActiveWeapon* action=static_cast<GameActionSetActiveWeapon*>(JE::g_World->NewAction(11));
			action->nextSlot=slot;
			action->previousSlot=(slot==2)+2;
			owner->QueueAction(action);
			return;
		}
		case 74: case 75: case 76: case 77: case 80:
		{
			//Same original read/read/lookup/lookup-only policy as Use72.
			JE::Net::UInt32 first,second;
			packet.ReadDword(first);
			packet.ReadDword(second);
			JE::GetEntityPtr(first);
			JE::GetEntityPtr(second);
			return;
		}
		case 58:
		{
			JE::Server::OnUserPacket(clientIndex,packet); //46EAB7, complete actual engine branch first.
			InitializeGameWorldAmmunition(); //470220 after nullable native IWorld -> GameWorld cast.
			return;
		}
		case 81: //46E235..46E2E0, Look from the real context menu.
		{
			JE::Net::UInt32 actorID,targetID;
			packet.ReadDword(actorID);
			packet.ReadDword(targetID);
			GameEntity* target=dynamic_cast<GameEntity*>(JE::GetEntityPtr(targetID));
			if (target)
			{
				JE::Script_Small* script=target->GetScript();
				cell reference;
				if (script && script->HasPublic("GetInfo")==1)
				{
					script->Run("GetInfo",&reference); //46E2AE ignores status, then reads the same output.
				}
				else reference=cell(target->stringRefs[1]); //Original GameEntity+C.
				if (reference) FloatingSpeechText(targetID,DWORD(reference),0,128,128,128);
			}
			return; //Original actor ID is consumed but otherwise unused.
		}
		case 79: //46E114..46E1FC; unlike78, this excludes creature targets.
		{
			JE::Net::UInt32 actorID, targetID;
			packet.ReadDword(actorID);
			packet.ReadDword(targetID);
			JE::Entity* actor = JE::GetEntityPtr(actorID);
			JE::Entity* target = JE::GetEntityPtr(targetID);
			if (actor && target)
			{
				if (target->entityClass == 3)
				{
					GameDoor* door = dynamic_cast<GameDoor*>(target);
					GameActionOperateDoor* action = new(std::nothrow) GameActionOperateDoor;
					action->SetDoor(door);
					actor->QueueAction(action);
				}
				else if (target->entityClass == 4)
				{
					GameActionOperateContainer* action = new(std::nothrow) GameActionOperateContainer;
					action->SetTarget(target);
					actor->QueueAction(action);
				}
			}
			return;
		}
		case 106: case 107: //46E8B5..46E8D9, original read-only prototype branches.
		{
			char name[260];
			JE::Net::UInt32 ignored;
			packet.ReadCString(name);
			packet.ReadDword(ignored);
			packet.ReadDword(ignored); //Both original reads use the same output word.
			return;
		}
		//83 is consumed above by ReceiveGameServerGiveRequest, before this switch.
		case 93: //46E63C..46E697, perk increment request.
		{
			JE::Net::UInt32 entityID, perk;
			packet.ReadDword(entityID);
			packet.ReadDword(perk);
			GameCreature* creature = dynamic_cast<GameCreature*>(JE::GetEntityPtr(entityID));
			if (creature) creature->IncrementPerk(LONG(perk));
			else JE::LogDebugString("GameNetwork::GAME_ID_ENTITY_PERK_INCREMENT - Could not find the needed entity.\n");
			return;
		}
		default:
			JE::Server::OnUserPacket(clientIndex,packet); //Actual46EAD0 engine fallback.
		}
	}
}
