#include "GameClient.hpp"
#include "gui/WorldContainerInterface.hpp"
#include "game/InventoryInterface.hpp"
#include "game/ActionbarInterface.hpp"
#include "gui/DebugWindow.hpp"
#include "EngineStringSystem.hpp"
#include "NetworkComponent.hpp"
#include "game/GameEntity.hpp"
#include "game/GameCreature.hpp"
#include "game/GameInventory.hpp"
#include "game/GameAmmo.hpp"
#include "game/GameWeapon.hpp"
#include "game/GameWorld.hpp"
#include "game/GameWorldCollections.hpp"
#include "entity/EntityEnvironment.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstdlib>
#include <new>
#include <stdexcept>

namespace F3
{
	namespace
	{
		GameInventory* RequireGameInventory(JE::Inventory* inventory)
		{
			GameInventory* game = dynamic_cast<GameInventory*>(inventory);
			return game;
		}

	}

	void GameWeapon::SetPrimaryAmmo(DWORD modifier, DWORD count) //412C10.
	{
		primaryModifier=modifier;
		primaryAmmoLoaded=count;
		if (count)
		{
			primaryAmmo=FindAmmunition(primaryCaliber,modifier);
			if (!primaryAmmo)
				JE::FatalError("Unable to look up ammo: Caliber: %d, modifier: %d");
		}
		else primaryAmmo=NULL;
	}

	void GameWeapon::SetSecondaryAmmo(DWORD modifier, DWORD count) //412C70.
	{
		secondaryModifier=modifier;
		secondaryAmmoLoaded=count;
		secondaryAmmo=FindAmmunition(secondaryCaliber,modifier);
		if (!secondaryAmmo)
			JE::FatalError("Unable to look up ammo: Caliber: %d, modifier: %d");
		if (!secondaryAmmoLoaded) secondaryAmmo=NULL;
	}

	void GameClient::RefreshInterface() //45E7F0, mandatory actual HUD owners.
	{
		//Reload the actual aggregate after each original callback.
		Interfaces()->inventory->Refresh(); //43F5E0.
		Interfaces()->actionbar->RefreshInventory(); //41C4C0.
		Interfaces()->container->Refresh(); //458710.
	}

	BYTE GameClient::FilterGamePacket(JE::Net::Packet& packet) //45E9A0.
	{
		if (!(g_pNetworkInstance->GetMode() & 1)) return 0;
		switch (packet.field0x30)
		{
		case 84:
			RefreshInterface();
			return 1;
		case 86:
			return 1;
		case 87:
			return 1;
		case 88:
			return 1;
		case 89:
			RefreshInterface();
			return 1;
		case 92:
			RefreshInterface();
			return 1;
		case 95:
			RefreshInterface();
			return 1;
		case 96:
			RefreshInterface();
			return 1;
		case 97:
			RefreshInterface();
			return 1;
		case 98:
			RefreshInterface();
			return 1;
		case 99:
			RefreshInterface();
			return 1;
		case 100:
			RefreshInterface();
			return 1;
		case 101:
			RefreshInterface();
			return 1;
		case 102:
			RefreshInterface();
			return 1;
		case 103:
			RefreshInterface();
			return 1;
		case 104:
			return 1;
		default:
			return JE::Client::FilterGamePacket(packet);
		}
	}

	void GameClient::OnGamePacket(JE::Net::Packet& packet) //461A10.
	{
		if (packet.field0x30 == 41)
			JE::LogDebugString("Game Client - Received GAME_ID_ITEM_EQUIP.\n");
		else if (packet.field0x30 == 42)
			JE::LogDebugString("Game Client - Received GAME_ID_ITEM_UNEQUIP.\n");
		if (FilterGamePacket(packet)) return;
		switch (packet.field0x30)
		{
		case 50:
		{
			//This game override reads two DWORDs directly; it does not use the
			//engine handler's preceding ownership BYTE.
			JE::Net::UInt32 ownerID, itemID;
			packet.ReadDword(ownerID);
			packet.ReadDword(itemID);
			JE::Entity* owner = JE::GetEntityPtr(ownerID);
			JE::Entity* item = JE::GetEntityPtr(itemID);
			if (owner && item)
			{
				GameInventory* inventory = dynamic_cast<GameInventory*>(owner->inventory);
				if (inventory) inventory->Remove(item);
				else JE::LogDebugString("ENGINE_ID_ITEM_REMOVE - Unable to retrieve the inventory in question.\n");
			}
			else JE::LogDebugString("ENGINE_ID_ITEM_REMOVE - Unable to find entity or the ammo.\n");
			return;
		}
		case 84: case 100: case 101:
			RefreshInterface();
			return;
		case 85:
		{
			JE::Net::UInt32 id, value, ignored;
			packet.ReadDword(id);
			packet.ReadDword(value);
			packet.ReadDword(ignored);
			JE::Entity* entity = JE::GetEntityPtr(id);
			if (!entity) JE::LogDebugString("Couldn't find target entity %d", id);
			else if (!dynamic_cast<GameEntity*>(entity))
				JE::LogDebugString("Target entity wasn't a game entity %s", entity->displayName.c_str());
			else
			{
				char buffer[12];
				_itoa(LONG(value), buffer, 10);
				std::string text(buffer);
				AddFloatingDamageText(id, text, 128, 0, 0); //461B5D..461B79, third wire word ignored.
			}
			return;
		}
		case 86: case 87: case 88: case 89:
		{
			JE::Net::UInt32 id, value;
			packet.ReadDword(id);
			packet.ReadDword(value);
			float speed;
			if (packet.field0x30 == 86) packet.ReadFloat(speed);
			GameEntity* entity = dynamic_cast<GameEntity*>(JE::GetEntityPtr(id));
			if (packet.field0x30 == 89)
			{
				//463B70 is called without a NULL test in this branch.
				entity->activeWeaponSlot = value;
				RefreshInterface();
			}
			else if (entity)
			{
				if (packet.field0x30 == 86) { entity->value90 = value; entity->value94 = speed; }
				else if (packet.field0x30 == 87) entity->maximumHealth = LONG(value);
				else entity->SetHealth(LONG(value));
			}
			return;
		}
		case 92:
		{
			JE::Net::UInt32 ownerID, ignoredID, caliber, modifier;
			packet.ReadDword(ownerID);
			packet.ReadDword(ignoredID); //The original reads, then discards this entity ID.
			packet.ReadDword(caliber);
			packet.ReadDword(modifier);
			GameAmmo* data = FindAmmunition(caliber, modifier);
			if (!data) JE::FatalError("GameClient::recv_packet() - Unable to look up ammo. Caliber %d, modifier %d",
				caliber, modifier);
			GameAmmo* clone = new(std::nothrow) GameAmmo;
			clone->CopyFrom(data);
			GameEntity* owner = dynamic_cast<GameEntity*>(JE::GetEntityPtr(ownerID));
			if (owner) owner->inventory->Add(clone);
			else JE::LogDebugString("GameClient - GAME_ID_ENTITY_ADD_AMMO_FROM_CACHE - ownerID is not a valid entity ID.\n");
			//Original invalid-owner path leaves the newly copied ammo allocated.
			return;
		}
		case 94:
		{
			JE::Net::UInt32 id, perk, rank;
			packet.ReadDword(id);
			packet.ReadDword(perk);
			packet.ReadDword(rank);
			GameCreature* creature = dynamic_cast<GameCreature*>(JE::GetEntityPtr(id));
			if (!creature)
			{
				JE::LogDebugString("GameNetwork::GAME_ID_ENTITY_PERK_SET - Unable to find the entity needed.\n");
				return;
			}
			creature->SetPerkRank(LONG(perk), LONG(rank));
			RefreshInterface();
			return;
		}
		case 95:
		{
			BYTE add;
			JE::Net::UInt32 ownerID, itemID, sourceID, slot;
			packet.ReadByte(add);
			packet.ReadDword(ownerID);
			packet.ReadDword(itemID);
			packet.ReadDword(sourceID);
			packet.ReadDword(slot);
			JE::Entity* owner = JE::GetEntityPtr(ownerID);
			JE::Entity* source = JE::GetEntityPtr(sourceID);
			JE::Entity* item = source->inventory->FindByID(itemID);
			if (add) owner->inventory->Add(item);
			else RequireGameInventory(owner->inventory)->Equip(item, slot, 1);
			RefreshInterface();
			return;
		}
		case 96:
		{
			BYTE add;
			JE::Net::UInt32 ownerID, itemID;
			packet.ReadByte(add);
			packet.ReadDword(ownerID);
			packet.ReadDword(itemID);
			JE::Inventory* inventory = JE::GetEntityPtr(ownerID)->inventory;
			JE::Entity* item = JE::GetEntityPtr(itemID);
			if (!item) item = inventory->FindByID(itemID);
			if (!add) RequireGameInventory(inventory)->Unequip(item);
			RefreshInterface();
			return;
		}
		case 97: case 98: case 99:
		{
			JE::Net::UInt32 id, value;
			packet.ReadDword(id);
			packet.ReadDword(value);
			GameJournal* journal = dynamic_cast<GameJournal*>(JE::g_World->journal);
			if (packet.field0x30 == 97) journal->OpenTask(id, value);
			else if (packet.field0x30 == 98) journal->CloseTask(id, value);
			else journal->AddEntry(id, value);
			RefreshInterface();
			return;
		}
		case 102: case 103:
		{
			JE::Net::UInt32 id, modifier, count;
			packet.ReadDword(id);
			packet.ReadDword(modifier);
			packet.ReadDword(count);
			JE::Entity* entity = JE::GetEntityPtr(id);
			GameWeapon* weapon = dynamic_cast<GameWeapon*>(entity);
			if (weapon)
			{
				if (packet.field0x30 == 102) weapon->SetPrimaryAmmo(modifier, count);
				else weapon->SetSecondaryAmmo(modifier, count);
				RefreshInterface();
			}
			else if (packet.field0x30 == 103)
				JE::LogDebugString("GAME_ID_WEAPON_SET_SECONDARY_AMMO - Unable to look up entity.\n");
			else if (!entity)
				JE::LogDebugString("GAME_ID_WEAPON_SET_PRIMARY_AMMO - Unable to look up entity ID.\n");
			else JE::LogDebugString("GAME_ID_WEAPON_SET_PRIMARY_AMMO - ID doesn't belong to a weapon. \n");
			return;
		}
		case 104:
			HideInterfaces(); //461E58 ->45E6C0.
			return;
		case 105:
		{
			char name[260];
			packet.ReadCString(name);
			throw std::logic_error("GameClient4621D0: actual store UI461960 remains unresolved");
		}
		case 108:
		{
			char name[260];
			JE::Net::UInt32 id;
			packet.ReadCString(name);
			packet.ReadDword(id);
			throw std::logic_error("GameClient461E06: actual WorldMap interface GameInterfaces+34/459E70 remains unresolved");
		}
		case 109:
		{
			JE::Net::UInt32 token;
			BYTE red, green, blue;
			packet.ReadDword(token);
			packet.ReadByte(red);
			packet.ReadByte(green);
			packet.ReadByte(blue);
			//46221A reads the published Network.client's window, not this receiver.
			JE::Client* client = dynamic_cast<JE::Client*>(g_pNetworkInstance->client);
			JE::DebugWindow* window = client->child0x8C;
			if (!window) return;
			std::string text;
			if (GetString(token, text))
				window->Write(std::string(text), red, green, blue); //462252..46229B, actual resolved text and copy lifetime.
			return; //4622A0, including failed string lookup; never fall through to the engine dispatcher.
		}
		default:
			JE::Client::OnGamePacket(packet); //Qualified engine fallback calls the virtual filter again.
		}
	}
}
