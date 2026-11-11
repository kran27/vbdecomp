#include "World.hpp"
#include "Map.hpp"
#include "network/Packet.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

namespace JE
{
	void World::PackState(Net::Packet& packet) //545930, original IWorldServer receiver is primary+4.
	{
		if (value38 != 3)
			FatalError("PackState() may only be called on a normal world.");
		packet.WriteDword(time48);
		packet.WriteDword(value4C);
		packet.WriteDword(value70);
		Net::UInt32 scaleBits;
		std::memcpy(&scaleBits, &value74, sizeof(scaleBits));
		packet.WriteDword(scaleBits);
		packet.WriteDword(value78);
		packet.WriteDword(value80);
		packet.WriteDword(value84);
		packet.WriteDword(value88);
		for (std::list<Entity*>::iterator i = g_createdEntities.begin(); i != g_createdEntities.end(); ++i)
		{
			Entity* entity = *i;
			packet.WriteDword(entity->entityID);
			packet.WriteDword(entity->entityClass);
			entity->PackState(packet);
		}
		packet.WriteSignedDword(0);
		for (WorldMaps::iterator i = maps.begin(); i != maps.end(); ++i)
		{
			Map* map = i->second;
			packet.WriteDword(map->idB4);
			map->PackState(packet);
		}
		packet.WriteSignedDword(0);
		packet.WriteDword(valueE0);
	}
	void World::UnpackState(Net::Packet& packet) //547E60, actual client+10.
	{
		Reset();
		packet.ReadDword(time48);
		packet.ReadDword(value4C);
		packet.ReadDword(value70);
		Net::UInt32 scaleBits;
		packet.ReadDword(scaleBits);
		std::memcpy(&value74, &scaleBits, sizeof(scaleBits));
		packet.ReadDword(value78);
		packet.ReadDword(value80);
		packet.ReadDword(value84);
		packet.ReadDword(value88);
		Net::UInt32 id;
		packet.ReadDword(id);
		while (id)
		{
			Net::UInt32 type;
			packet.ReadDword(type);
			Entity* entity = NewEntity(type);
			entity->UnpackState(id, packet);
			packet.ReadDword(id);
		}
		packet.ReadDword(id);
		while (id)
		{
			Map* map = NewMap();
			map->UnpackState(id, packet);
			map->GetOctree();
			{
				std::string name(map->resourceName.c_str());
				maps[name] = map;
			}
			packet.ReadDword(id);
		}
		value38 = 3;
		packet.ReadDword(valueE0);
	}

}
