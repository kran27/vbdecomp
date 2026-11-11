#include "Map.hpp"
#include "model/CommonException.hpp"
#include "network/Packet.hpp"
#include <cstring>

namespace JE
{
	void Map::PackState(Net::Packet& packet) //56D7F0.
	{
		if (state != 3)
			throw ::Common::Exception("PackState() method may only be called on normal maps.");
		const char* text = resourceName.c_str();
		packet.Write(text, std::strlen(text) + 1);
		packet.WriteDword(value28);
		text = octreeName.c_str();
		packet.Write(text, std::strlen(text) + 1);
		text = gridName.c_str();
		packet.Write(text, std::strlen(text) + 1);
		text = areaTextureName.c_str();
		packet.Write(text, std::strlen(text) + 1);
		text = scriptName.c_str();
		packet.Write(text, std::strlen(text) + 1);
		packet.WriteDword(visibilityOwners);
		for (std::list<Entity*>::iterator i = activeEntities.begin(); i != activeEntities.end(); ++i)
			packet.WriteDword((*i)->entityID);
		packet.WriteSignedDword(0); //45F0B0: four-byte terminator.
	}

	void Map::UnpackState(DWORD id, Net::Packet& packet) //572F10.
	{
		if (state)
			throw ::Common::Exception("UnpackState() method may only be called on invalid maps.");
		char text[256];
		packet.ReadCString(text); resourceName=text;
		packet.ReadDword(value28);
		packet.ReadCString(text); octreeName=text;
		packet.ReadCString(text); gridName=text;
		packet.ReadCString(text); areaTextureName=text;
		packet.ReadCString(text); scriptName=text;
		idB4=id;
		packet.ReadDword(visibilityOwners);
		Net::UInt32 word;
		packet.ReadDword(word);
		while (word)
		{
			Entity* entity=GetEntityPtr(word);
			activeEntities.push_back(entity);
			if (!entity->inventoryOwnerID) entity->AttachToOctree(GetOctree());
			packet.ReadDword(word);
		}
		state=3; //Raw publication; original does not call ChangeState/AddEntity.
	}
}
