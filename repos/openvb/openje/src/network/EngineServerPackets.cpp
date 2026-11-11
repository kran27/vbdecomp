#include "EngineServer.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/ActionMoveTo.hpp"
#include "entity/Entity.hpp"
#include "entity/Inventory.hpp"
#include "entity/Action.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/World.hpp"
#include "entity/Map.hpp"
#include "entity/WorldCollections.hpp"
#include "scripting/ScriptSmall.hpp"
#include "model/CommonException.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstdio>
#include <stdexcept>
#include <cstdlib>

namespace JE
{
	void Server::PrepareStart(JE::Net::UInt32 index,Net::Packet& packet)
	{
		const JE::Net::UInt32 identity = AllocateClientID();
		packet.WriteDword(identity);
		static_cast<Net::EngineServerClientConnection*>(connections[index])->field0x4168 = identity;
		LogDebugString("Server: New client connected with an id of %u\n",identity);
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world = g_World;
		world->PackState(packet);
	}

	JE::Net::UInt8 Server::OnDisconnectRequest(JE::Net::UInt32 index,Net::Packet&) //555380.
	{
		Net::EngineServerClientConnection* connection =
			static_cast<Net::EngineServerClientConnection*>(connections[index]);
		//Original reloads the head after Destroy and never advances past a
		//nonmatching head. Keep that policy, including its nonprogress cases.
		while (g_createdEntities.begin() != g_createdEntities.end())
		{
			Entity* entity = *g_createdEntities.begin();
			if (entity->visibilityOwnerFlag == connection->field0x4168) entity->Destroy();
		}
		return 1;
	}
	void Server::OnUserPacket(JE::Net::UInt32 clientIndex,Net::Packet& packet)
	{
		switch (packet.field0x30)
		{
		case 9: //556EF8..556F9D, CreateEntity request from589820.
		{
			char resource[260];
			Net::UInt32 mapID;
			D3DXVECTOR3 position(0,0,0), rotation(0,0,0);
			Net::UInt8 alternateCreature;
			packet.ReadCString(resource);
			packet.ReadDword(mapID);
			packet.ReadFloat(position.x);
			packet.ReadFloat(position.y);
			packet.ReadFloat(position.z);
			packet.ReadBool(alternateCreature);
			Entity* entity = CreateEntity(resource,0,alternateCreature);
			MoveEntityToMap(entity,mapID,position,rotation,0);
			return;
		}
		case 10: //556FA7..556FDD; client-owned entities cannot be destroyed here.
		{
			Net::UInt32 entityID;
			packet.ReadDword(entityID);
			Entity* entity = GetEntityPtr(entityID);
			if (entity && !entity->visibilityOwnerFlag) entity->Destroy();
			return;
		}
		case 15: //556FE2..557061, script-name access only; no script loading.
		{
			Net::UInt32 entityID;
			char script[68];
			packet.ReadDword(entityID);
			packet.ReadCString(script);
			Entity* entity = GetEntityPtr(entityID);
			if (!entity)
			{
				SendConsoleMessage(clientIndex,"Invalid entity.");
				return;
			}
			const char* currentScript = entity->GetScriptName(); //436250.
			if (currentScript && *currentScript)
				SendConsoleMessage(clientIndex,"Entity already has a script.");
			else
				entity->SetScriptName(script); //426510: CString length, no callbacks.
			return;
		}
		case 26: //5571C9..55726C, then original shared idle tail55764C.
		{
			Net::UInt32 entityID, group, file;
			Net::UInt16 loops;
			Net::UInt8 reverse;
			packet.ReadDword(entityID);
			packet.ReadDword(group);
			packet.ReadDword(file);
			packet.ReadWord(loops);
			packet.ReadBool(reverse);
			Entity* entity = GetEntityPtr(entityID);
			if (!entity)
			{
				SendConsoleMessage(clientIndex,"Server could not find the specified entity.");
				return;
			}
			entity->ClearActions();
			ActionAnim* action = static_cast<ActionAnim*>(g_World->NewAction(6));
			action->group = group;
			action->file = file;
			action->loops = loops;
			action->reverse = reverse;
			entity->QueueAction(action);
			entity->QueueAction(g_World->NewAction(1));
			return;
		}
		case 37: return; //Original empty case25h.
		case 40: //557066..5570EB, Encyclopedia entry's script, with the requesting client argument.
		{
			Net::UInt32 entryID;
			packet.ReadDword(entryID);
			EncyclopediaEntry* entry = g_World->GetEncyclopedia()->FindEntry(entryID);
			if (entry)
			{
				Script_Small* script = GetWorldServerInstance()->NewScript(0);
				if (script->Load(entry->valueC.c_str(), 1))
				{
					cell result;
					cell argument = cell(clientIndex);
					script->Run("Run", &result, 1, &argument); //Original literal692A30 is "Run".
				}
				delete script; //Both ordinary Load outcomes delete; a thrown callback is not caught.
			}
			return;
		}
		case 45: //5570F0..5571C4, selected WorldMap location's OnEnter script.
		{
			char mapName[260];
			Net::UInt32 locationID;
			packet.ReadCString(mapName);
			packet.ReadDword(locationID);
			WorldMapLocation* location = g_World->GetWorldMap(mapName)->FindLocation(locationID);
			if (!location) return;
			std::string scriptName;
			location->GetOnEnterScript(scriptName);
			if (!scriptName.empty())
			{
				Script_Small* script = GetWorldServerInstance()->NewScript(0);
				if (script->Load(scriptName.c_str(), 1))
				{
					cell result;
					script->Run("OnEnter", &result);
				}
				delete script; //Deletion precedes the copied script-name lifetime's end.
			}
			return;
		}
		case 49: ReadInventoryTransferRequest(packet); return;
		case 50: //557432..5574AC; captured request IDs are published even on lookup failure.
		{
			Net::UInt32 ownerID, itemID;
			packet.ReadDword(ownerID);
			packet.ReadDword(itemID);
			BYTE failure = 0;
			Entity* owner = GetEntityPtr(ownerID);
			if (!owner) failure = 1;
			Entity* item = NULL;
			if (owner)
			{
				item = owner->inventory->FindByID(itemID);
				if (!item) failure = 1;
			}
			if (!failure) owner->inventory->Remove(item);
			PublishInventoryRemoval(ownerID, itemID, failure);
			return;
		}
		case 51: ReadInventoryDropRequest(packet); return;
		case 53:
		{
			char name[260];
			packet.ReadCString(name);
			//556230 only destroys its by-value string parameter. The shipped
			//StartDialogue test request has no dialogue operation behind it.
			std::string unusedName(name);
			return;
		}
		case 58: StartNewGame(); return; //5575D0, original case3A consumes no payload.
		case 59: //5575DA..557662; dispatch stays selected across ClearActions.
		{
			Net::UInt32 entityID;
			packet.ReadDword(entityID);
			D3DXVECTOR3 position(0,0,0);
			packet.ReadFloat(position.x);
			packet.ReadFloat(position.y);
			packet.ReadFloat(position.z);
			Entity* entity = GetEntityPtr(entityID);
			if (!entity) return;
			entity->ClearActions();
			entity->SetPosition(position);
			entity->QueueAction(GetWorldInstance()->NewAction(1));
			return;
		}
		case 60: //557667..5576FC; no packet-kind reload after callbacks.
		{
			Net::UInt32 entityID;
			packet.ReadDword(entityID);
			D3DXVECTOR3 position(0,0,0);
			packet.ReadFloat(position.x);
			packet.ReadFloat(position.y);
			packet.ReadFloat(position.z);
			Entity* entity = GetEntityPtr(entityID);
			if (!entity) return;
			entity->ClearActions();
			ActionMoveTo* action = static_cast<ActionMoveTo*>(GetWorldInstance()->NewAction(2));
			action->destination.x = position.x;
			action->destination.z = position.z;
			action->destination.y = position.y;
			action->flag34 = 1;
			entity->QueueAction(action);
			return;
		}
		case 61: //5576FE..55778D; unlike Warp, the original does not test LoadMap's result.
		{
			char name[260];
			packet.ReadCString(name);
			Map* map = GetWorldServerInstance()->LoadMap(name);
			{
				std::string messageName(name);
				BroadcastLoadedMap(messageName, map->idB4);
			}
			SendActiveMap(map->idB4, 0, clientIndex);
			return;
		}
		case 64:
			packet.ReadDword(static_cast<Net::EngineServerClientConnection*>(
				connections[clientIndex])->field0x416C);
			return;
		case 65: //556EA0 branch: CString/signed-DWORD ->SetGlobal5566B0.
		{
			char name[260];
			JE::Net::UInt32 value;
			packet.ReadCString(name);
			packet.ReadDword(value);
			SetGlobal(name,JE::Net::Int32(value));
			return;
		}
		case 54:
			{
				JE::Net::UInt8 id, revision;
				JE::Net::UInt32 replyID, participantID;
				packet.ReadByte(id);
				packet.ReadByte(revision);
				packet.ReadDword(replyID);
				packet.ReadDword(participantID);
				static_cast<DialogueManager*>(dialogue)->SelectReply(id, revision, replyID, participantID);
				return;
			}
		case 56:
			static_cast<DialogueManager*>(dialogue)->EndAllDialogues();
			return;
		case 57:
			{
				JE::Net::UInt8 id;
				packet.ReadByte(id);
				static_cast<DialogueManager*>(dialogue)->RequestEndDialogue(id);
				return;
			}
		case 70: //557811..557950; native map load and caller-owned name lifetime.
		{
			char name[260];
			packet.ReadCString(name);
			Map* map = GetWorldServerInstance()->LoadMap(name);
			if (!map)
			{
				SendConsoleMessage(clientIndex, "Unable to load map.");
				return;
			}
			SendConsoleMessage(clientIndex, "Warping to new map.");
			const Net::UInt32 mapID = map->idB4;
			{
				const std::string messageName(name);
				BroadcastLoadedMap(messageName,mapID);
			}
			Net::EngineServerClientConnection* connection =
				static_cast<Net::EngineServerClientConnection*>(connections[clientIndex]);
			Net::UInt32 firstEntityID = 0;
			LONG moved = 0;
			for (std::list<Entity*>::iterator entry = g_createdEntities.begin();
				entry != g_createdEntities.end(); ++entry)
			{
				Entity* entity = *entry;
				if (entity->visibilityOwnerFlag == connection->field0x4168)
				{
					if (!moved) firstEntityID = entity->entityID;
					entity->ClearActions();
					MoveEntityToMapEntry(entity,mapID,0,5-moved,0);
					++moved;
				}
			}
			SendPlayerID(clientIndex,firstEntityID,1);
			return;
		}
		case 67: //557797..55780C, join request; both template string reads remain separate.
		{
			SendCreatureFile(clientIndex, GetCharacterTemplate());
			Entity* entity = CreateEntity(GetCharacterTemplate(), 1, 0);
			AssignEntityClient(entity->entityID, clientIndex);
			JoinParty(entity);
			const DWORD mapID = g_World->GetCurrentMapID(); //41D050, no server-mode check here.
			MoveEntityToMapEntry(entity, mapID, 0, 5, 0);
			LogDebugString("Client %d joined the game.  Assigned entity %d.\n",
				clientIndex, entity->entityID);
			return;
		}
		default:
			if (packet.field0x30>=9)
			{
				char address[64]; //Actual default uses Address5ACAF0 with includePort1.
				packet.address.Format(address,sizeof(address),1);
				LogDebugString("Server received packet with unknown id %d from %s\r\n",
					packet.field0x30,address);
			}
			return; //Includes actual sequenced keepalive7: no game operation.
		}
	}
}
