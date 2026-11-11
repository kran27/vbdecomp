#include <winsock2.h>
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "network/EngineServer.hpp"
#include "network/Connection.hpp"
#include <cstring>

namespace JE
{
	void Server::SendWorldMapArea(const char* filename, DWORD id, DWORD flags) //554C30.
	{
		Net::Packet packet;
		packet.field0x30 = 46;
		packet.Write(filename,static_cast<DWORD>(std::strlen(filename)+1));
		packet.WriteSignedDword(static_cast<Net::Int32>(id));
		packet.WriteSignedDword(static_cast<Net::Int32>(flags));
		Broadcast(packet, 1);
	}

	void Server::SetWorldMapAreaVisible(const char* filename, DWORD id, BYTE active) //555D80.
	{
		IWorld* world=g_World;
		WorldMap* map=world->GetWorldMap(filename);
		if (!map) return;
		WorldMapLocation* area=map->FindLocation(id);
		if (!area) return;
		if (active)
		{
			DWORD flags=area->values[0]|1;
			area->values[0]=flags;
			SendWorldMapArea(filename,id,flags);
		}
		else
		{
			DWORD flags=area->values[0]&~DWORD(1);
			area->values[0]=flags;
			SendWorldMapArea(filename,id,flags);
		}
	}
	void Server::SetWorldMapAreaExplorable(const char* filename, DWORD id, BYTE active) //555E00.
	{
		IWorld* world=g_World;
		WorldMap* map=world->GetWorldMap(filename);
		if (!map) return;
		WorldMapLocation* area=map->FindLocation(id);
		if (!area) return;
		if (active)
		{
			DWORD flags=area->values[0]|2;
			area->values[0]=flags;
			SendWorldMapArea(filename,id,flags);
		}
		else
		{
			DWORD flags=area->values[0]&~DWORD(2);
			area->values[0]=flags;
			SendWorldMapArea(filename,id,flags);
		}
	}
	void Server::SetWorldMapAreaExplored(const char* filename, DWORD id, BYTE active) //555E80.
	{
		IWorld* world=g_World;
		WorldMap* map=world->GetWorldMap(filename);
		if (!map) return;
		WorldMapLocation* area=map->FindLocation(id);
		if (!area) return;
		if (active)
		{
			DWORD flags=area->values[0]|4;
			area->values[0]=flags;
			SendWorldMapArea(filename,id,flags);
		}
		else
		{
			DWORD flags=area->values[0]&~DWORD(4);
			area->values[0]=flags;
			SendWorldMapArea(filename,id,flags);
		}
	}

	void Server::SetEncyclopediaData(DWORD clientIndex, DWORD dataID, const char* filename) //555D70 ->554B40.
	{
		Net::Packet notification;
		notification.field0x30 = 42;
		notification.WriteSignedDword(static_cast<Net::Int32>(dataID));
		notification.Write(filename,static_cast<DWORD>(std::strlen(filename)+1));
		SendToClient(clientIndex, notification, 1);
	}
}
