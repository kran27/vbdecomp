#include "Map.hpp"
#include "MapMetadata.hpp"
#include "resources/ResourceSystem.hpp"
#include <new>

namespace JE
{
	MapEntryPoint* Map::NewEntryPoint() { return new MapEntryPoint; }
	void Map::ClearEntryPoints()
	{
		std::map<BYTE,MapEntryPoint*>::iterator i = entryPoints.begin();
		while (i != entryPoints.end())
		{
			MapEntryPoint* point = i->second;
			i = entryPoints.erase(i); //571000 unlinks before releasing the payload.
			delete point;
		}
	}
	BYTE Map::ReadEntryPoint(Common::Chunk&, Resources::ResourceMemoryStream* stream)
	{
		BYTE id = 0;
		MapEntryPoint* point = NewEntryPoint();
		if (!Resources::g_pResourceSystem->ReadMemory(&id,1,stream)) { delete point; return 0; }
		for (LONG i = 0; i < 6; ++i)
			if (!Resources::g_pResourceSystem->ReadMemory(&point->positions[i].values[0],4,stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&point->positions[i].values[1],4,stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&point->positions[i].values[2],4,stream))
			{ delete point; return 0; }
		for (LONG i = 0; i < 6; ++i)
			if (!Resources::g_pResourceSystem->ReadMemory(point->values+i,4,stream)) { delete point; return 0; }
		SetEntryPoint(id, point); //Replaces without freeing an old same-ID payload.
		return 1;
	}
	BYTE Map::ReadNote(Common::Chunk&, Resources::ResourceMemoryStream* stream)
	{
		float x, y, z;
		if (Resources::g_pResourceSystem->ReadMemory(&x,4,stream)
			&& Resources::g_pResourceSystem->ReadMemory(&y,4,stream)
			&& Resources::g_pResourceSystem->ReadMemory(&z,4,stream))
		{
			std::string name;
			DWORD value;
			if (!Common::ReadString(name,stream)) return 0;
			if (!Resources::g_pResourceSystem->ReadMemory(&value,4,stream)) return 0;
			AreaMapNote note;
			note.name = name;
			note.value30 = LONG(value);
			note.position[0] = x;
			note.position[1] = y;
			note.position[2] = z;
			mapNotes.push_back(note);
			return 1;
		}
		return 0;
	}
	BYTE Map::ReadHeightFog(Common::Chunk&, Resources::ResourceMemoryStream* stream)
	{
		BYTE enabled, red, green, blue;
		if (!Resources::g_pResourceSystem->ReadMemory(&enabled,1,stream)) return 0;
		flag178 = enabled != 0; //56CD94 confirms BYTE test despite decompiler's reused argument.
		if (!Resources::g_pResourceSystem->ReadMemory(&red,1,stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&green,1,stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&blue,1,stream)) return 0;
		value17C = ((((DWORD(-1) << 8) | red) << 8) | green) << 8 | blue;
		if (!Resources::g_pResourceSystem->ReadMemory(&value180,4,stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&value184,4,stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&value188,4,stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&value18C,4,stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&value190,4,stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&value194,4,stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&value198,4,stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&value19C,4,stream)) return 0;
		value1A0 = value17C;
		value1A8 = value18C;
		value1A4 = value180;
		value1AC = value19C;
		return 1;
	}
}
