#include "EngineServer.hpp"
#include "entity/World.hpp"
#include "entity/Map.hpp"
#include "startup/NetworkComponent.hpp"
#include "resources/ResourceSystem.hpp"
#include "model/CommonException.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstdio>
#include <cstring>

namespace JE
{
	void Server::SendCreatureFile(JE::Net::UInt32 clientIndex, const char* resource)
	{
		//555170 transfers the actual selected resource bytes, not a reconstructed
		//or serialized substitute for characterTemplate.
		Resources::ResourceData* data=Resources::g_pResourceSystem->Demand(resource,1);
		if (!data) FatalError("Attempt to send non-existent creature file to client.");
		DWORD size=data->size;
		if (size>=8192) FatalError("Attempt to send over-sized creature file to client.");
		Net::Packet notification;
		notification.field0x30=71;
		notification.Write(resource,static_cast<DWORD>(std::strlen(resource)+1));
		notification.WriteSignedDword(static_cast<Net::Int32>(size));
		notification.Write(data->data,size);
		Resources::g_pResourceSystem->ReleaseResource(data);
		Net::Server::SendToClient(clientIndex,notification,1);
	}

	Entity* Server::CreateEntity(const char* resource, JE::Net::UInt8 loadModel,
		JE::Net::UInt8 alternateCreature)
	{
		if (!(F3::g_pNetworkInstance->GetMode()&1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		DWORD type=world->ClassifyEntity(resource,alternateCreature);
		if (!(F3::g_pNetworkInstance->GetMode()&1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* nextWorld=g_World;
		Entity* entity=nextWorld->NewEntity(type);
		entity->Create(resource,loadModel,0);
		SendEntityCreation(resource,entity->entityID,loadModel,alternateCreature);
		return entity;
	}
	void Server::SendEntityCreation(const char* resource,DWORD entityID,BYTE loadModel,
		BYTE alternateCreature) //553600, broadcast after Create completes.
	{
		Net::Packet notification;
		notification.field0x30=9;
		notification.Write(resource,static_cast<DWORD>(std::strlen(resource)+1));
		notification.WriteDword(entityID);
		notification.WriteByte(loadModel);
		notification.WriteByte(alternateCreature);
		Broadcast(notification,1);
	}

	void Server::SendEntityClientAssignment(DWORD entityID,DWORD clientID) //553CF0.
	{
		Net::Packet notification;
		notification.field0x30=14;
		notification.WriteDword(entityID);
		notification.WriteDword(clientID);
		Broadcast(notification,1);
	}

	void Server::MoveEntityToMapEntry(Entity* entity, JE::Net::UInt32 mapID,
		JE::Net::UInt8 entryID, JE::Net::Int32 entrySlot, JE::Net::UInt8 flag)
	{
		if (!(F3::g_pNetworkInstance->GetMode()&1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world=g_World;
		Map* map=world->GetMap(mapID);
		PlaceEntityAtMapEntry(entity,map,entryID,entrySlot,flag);
	}

	void Server::SendPlayerID(JE::Net::UInt32 clientIndex, JE::Net::UInt32 entityID,
		JE::Net::UInt8 flag)
	{
		Net::Packet notification; //555050, original targeted message69.
		notification.field0x30=69;
		notification.WriteDword(entityID);
		notification.WriteByte(flag);
		Net::Server::SendToClient(clientIndex,notification,1);
	}

	void Server::StartNewGame() //556B60.
	{
		Net::Packet notification;
		notification.field0x30=58;
		const char* filename=startMap.c_str();
		notification.Write(filename,static_cast<DWORD>(std::strlen(filename)+1));
		Broadcast(notification,1);
		if (!(F3::g_pNetworkInstance->GetMode()&1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* resetWorld=g_World;
		resetWorld->Reset();
		if (!(F3::g_pNetworkInstance->GetMode()&1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* mapWorld=g_World;
		Map* map=mapWorld->LoadMap(startMap.c_str());
		if (!map)
			FatalError("Invalid start map %s when trying to start a new game.",startMap.c_str());
		Net::UInt32 clientIndex=0;
		LONG entrySlot=5;
		while (clientIndex<field0x60)
		{
			Net::EngineServerClientConnection* client=
				static_cast<Net::EngineServerClientConnection*>(connections[clientIndex]);
			if (static_cast<LONG>(client->state)>0)
			{
				SendCreatureFile(clientIndex,characterTemplate.c_str());
				Entity* entity=CreateEntity(characterTemplate.c_str(),1,1);
				AssignEntityClient(entity->entityID,clientIndex);
				JoinParty(entity);
				MoveEntityToMapEntry(entity,map->idB4,entryPoint,entrySlot,0);
				char name[16];
				std::sprintf(name,"bob %d",static_cast<int>(client->field0x4168));
				entity->SetDisplayName(std::string(name));
				SendPlayerID(clientIndex,entity->entityID,1);
			}
			++clientIndex;
			--entrySlot;
		}
	}

	Entity* Server::SpawnNewGamePlayer(JE::Net::UInt32 clientIndex, Map* map,
		JE::Net::Int32 entrySlot)
	{
		//Explicit556CF0..556E25 factor from556B60. Full new-game reset and
		//load-map dispatch remain with the outer caller. The original parent
		//passes5-index and decrements that slot even for inactive connections.
		Net::EngineServerClientConnection* client=
			static_cast<Net::EngineServerClientConnection*>(connections[clientIndex]);
		if (static_cast<LONG>(client->state)<=0) return NULL;
		SendCreatureFile(clientIndex,characterTemplate.c_str());
		Entity* entity=CreateEntity(characterTemplate.c_str(),1,1);
		AssignEntityClient(entity->entityID,clientIndex);
		JoinParty(entity);
		MoveEntityToMapEntry(entity,map->idB4,entryPoint,entrySlot,0);
		char name[16];
		std::sprintf(name,"bob %d",static_cast<int>(client->field0x4168));
		entity->SetDisplayName(std::string(name));
		SendPlayerID(clientIndex,entity->entityID,1);
		return entity; //Host-visible result of this explicitly named source factor.
	}
}
