#include "EngineServer.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/World.hpp"
#include "entity/Map.hpp"
#include "entity/EntityEnvironment.hpp"
#include "model/CommonException.hpp"
#include <cstring>
#include <stdexcept>

namespace JE
{
	void Server::SendConsoleMessage(JE::Net::UInt32 clientIndex, const char* text)
	{
		Net::Packet notification; //554F70, real targeted console message68.
		notification.field0x30 = 68;
		notification.Write(text, static_cast<Net::UInt32>(std::strlen(text) + 1));
		Net::Server::SendToClient(clientIndex,notification,1); //Actual5A09D0, enabled policy stays in that owner.
	}

	void Server::BroadcastLoadedMap(const std::string& name, JE::Net::UInt32 mapID)
	{
		Net::Packet notification; //552E20; name is borrowed, caller owns its temporary.
		notification.field0x30 = 61;
		const char* filename=name.c_str();
		notification.Write(filename, static_cast<Net::UInt32>(std::strlen(filename) + 1));
		notification.WriteDword(mapID);
		Broadcast(notification, 1);
	}

	void Server::SendActiveMap(JE::Net::UInt32 mapID, JE::Net::UInt8 broadcast,
		JE::Net::UInt32 clientIndex)
	{
		Net::Packet notification; //552F20 writes an ID, even though the remote63 reader expects a name.
		notification.field0x30 = 63;
		notification.WriteDword(mapID);
		if (broadcast) Broadcast(notification, 1);
		else Net::Server::SendToClient(clientIndex,notification,1);
	}

	void Server::WarpClient(JE::Net::UInt32 clientIndex, const char* name)
	{
		//557820..55794B: load before replying or moving any client-owned entity.
		if (!(GetEntityEnvironment().NetworkMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		if (!g_World) throw std::logic_error("Server557820: original unchecked World required");
		Map* map = g_World->LoadMap(name);
		if (!map)
		{
			SendConsoleMessage(clientIndex, "Unable to load map.");
			return;
		}
		SendConsoleMessage(clientIndex, "Warping to new map.");
		const JE::Net::UInt32 mapID = map->idB4;
		{
			const std::string packetName(name);
			BroadcastLoadedMap(packetName, mapID);
		}
		Net::EngineServerClientConnection* connection =
			static_cast<Net::EngineServerClientConnection*>(connections[clientIndex]);
		JE::Net::UInt32 firstEntityID = 0;
		LONG moved = 0;
		for (std::list<Entity*>::iterator entry = g_createdEntities.begin();
			entry != g_createdEntities.end(); ++entry)
		{
			Entity* entity = *entry;
			if (entity->visibilityOwnerFlag == connection->field0x4168)
			{
				if (!moved) firstEntityID = entity->entityID;
				entity->ClearActions();
				MoveEntityToMapEntry(entity, mapID, 0, 5 - moved, 0);
				++moved;
			}
		}
		SendPlayerID(clientIndex, firstEntityID, 1);
	}

	void Server::MoveEntityToNamedMapEntry(Entity* entity, const char* mapName,
		JE::Net::UInt8 entryID, JE::Net::Int32 entrySlot, JE::Net::UInt8 flag)
	{
		if (!(F3::g_pNetworkInstance->GetMode()&1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		Map* map=world->LoadMap(mapName);
		PlaceEntityAtMapEntry(entity,map,entryID,entrySlot,flag);
	}

	void Server::PlaceEntityAtMapEntry(Entity* entity, Map* map, BYTE entryID,
		Net::Int32 entrySlot, BYTE flag) //556270.
	{
		D3DXVECTOR3 position(0,0,0), rotation(0,0,0);
		map->GetEntryPosition(entryID,entrySlot,position,rotation);
		AddEntityToMap(entity,map,position,rotation,flag);
	}
}
