#include <winsock2.h>
#include "GameActionGive.hpp"
#include "GameActionMoveTo.hpp"
#include "GameCreature.hpp"
#include "GameInventory.hpp"
#include "GameInterfaces.hpp"
#include "InventoryInterface.hpp"
#include "startup/NetworkComponent.hpp"
#include "gui/DebugWindow.hpp"
#include "gui/WorldContainerInterface.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineServer.hpp"
#include "network/Packet.hpp"
#include <math.h>
#include <new>

namespace JE
{
	//405329 retains cleanup/return after this call. The original declaration
	//does not have the older diagnostic adapter's noreturn attribute.
	void __cdecl FatalError(const char* format, ...);
}

namespace F3
{
	GameActionGive::GameActionGive() //4052A0.
		: completed(0), giver(NULL), giverID(0), target(NULL), targetID(0), itemID(0)
	{
		kind = 1;
	}
	BYTE GameActionGive::IsComplete() const { return completed; }
	void GameActionGive::Cancel() {}
	int GameActionGive::Finish(DWORD) { return 0; }
	void GameActionGive::SetTarget(JE::Entity* entity) //4052D0.
	{
		target = entity;
		if (entity) targetID = entity->entityID;
	}
	void GameActionGive::SetItem(DWORD entityID) //4052F0.
	{
		itemID = entityID;
	}
	int GameActionGive::Start(DWORD) //405300.
	{
		giver = dynamic_cast<GameCreature*>(owner);
		if (giver) giverID = giver->entityID;
		else JE::FatalError("GameActionGive::OnCreate() - Giver is not a GameCreature.");
		return 0;
	}
	int GameActionGive::Update(DWORD, DWORD budget) //405370.
	{
		int used = 0;
		giver = dynamic_cast<GameCreature*>(JE::GetEntityPtr(giverID));
		target = JE::GetEntityPtr(targetID);
		if (!giver || !target)
		{
			completed = 1;
			return 0;
		}
		//4053D8..405423 retains the y/z float stores and extended x difference.
		const float dz = giver->position.z - target->position.z;
		const float dy = giver->position.y - target->position.y;
		const double dx = double(giver->position.x) - target->position.x;
		if (sqrt(dx * dx + double(dy) * dy + double(dz) * dz) >= 5.0)
		{
			GameActionMoveTo* move = new(std::nothrow) GameActionMoveTo;
			move->SetTarget(target);
			move->stopDistance = 5;
			giver->QueueAction(move);
			GameActionGive* retry = new(std::nothrow) GameActionGive;
			retry->SetTarget(target);
			retry->SetItem(itemID);
			giver->QueueAction(retry);
			completed = 1;
			return 0;
		}
		if (budget < 12) return 0;
		if (itemID)
		{
			used = 12;
			GameInventory* source = dynamic_cast<GameInventory*>(giver->inventory);
			GameInventory* destination = dynamic_cast<GameInventory*>(target->inventory);
			if (source && destination)
			{
				JE::Entity* item = source->FindByID(itemID);
				if (item)
				{
					//The original requires these actual owners and reloads the global
					//after transfer. Destination RTTI is only a validity predicate.
					static_cast<JE::Server*>(g_pNetworkInstance->server)->AddItemToInventory(target, item);
					JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
					client->child0x8C->Write(std::string("Item Given"), 128, 128, 128);
					GameInterfaces* interfaces = static_cast<GameInterfaces*>(client->child0xA0);
					InventoryInterface* inventory = interfaces->inventory;
					interfaces->container->Refresh();
					inventory->Refresh();
				}
			}
		}
		completed = 1;
		return used;
	}

	BYTE ReceiveGameServerGiveRequest(JE::Net::Packet& packet)
	{
		if (packet.field0x30 != 83) return 0;
		JE::Net::UInt32 actorID, targetID, itemID;
		packet.ReadDword(actorID);
		packet.ReadDword(targetID);
		packet.ReadDword(itemID);
		JE::Entity* actor = JE::GetEntityPtr(actorID);
		JE::Entity* target = JE::GetEntityPtr(targetID);
		if (actor && target)
		{
			GameActionGive* action = new(std::nothrow) GameActionGive;
			action->SetTarget(target);
			action->SetItem(itemID);
			actor->QueueAction(action); //46E3E8; original does not clear actions.
		}
		return 1; //Factor consumption flag, not transfer success or an original return.
	}
}
