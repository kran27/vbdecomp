#include "Map.hpp"
#include "MapWater.hpp"
#include "resources/ResourceSystem.hpp"
#include "graphics/StringFormat.hpp"

namespace JE
{
	MapWaterTile::MapWaterTile() : position(0, 0, 0)
	{
		//56EB70 only initializes position, three strings and the real mesh.
	}

	BYTE Map::ReadWater(Common::Chunk&, Resources::ResourceMemoryStream* stream)
	{
		MapWaterConfiguration configuration;
		DWORD id = static_cast<DWORD>(waterConfigurations.size());
		std::string name;
		std::string presetName;
		FormatString(name, "Water%d", static_cast<LONG>(id));
		if (!Common::ReadString(presetName, stream)) return 0;
		configuration.presetName = presetName;
		configuration.name = name;
		configuration.id = id;
		if (!Resources::g_pResourceSystem->ReadMemory(&configuration.value48, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.value4C, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.value50, 4, stream)) return 0;
		BYTE value;
		if (!Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
		configuration.flag3C = value != 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
		configuration.flag3D = value != 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
		configuration.flag3E = value != 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
		configuration.flag3F = value != 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&configuration.value40, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.value54, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.value44, 4, stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&configuration.rippleMinimumLocation.x, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.rippleMinimumLocation.y, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.rippleMaximumLocation.x, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.rippleMaximumLocation.y, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.rippleMinimumAmount, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.rippleMaximumAmount, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.rippleMinimumInterval, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.rippleMaximumInterval, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.lineMinimumOrigin.x, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.lineMinimumOrigin.y, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.lineMaximumOrigin.x, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.lineMaximumOrigin.y, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.lineMinimumDirection.x, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.lineMinimumDirection.y, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.lineMaximumDirection.x, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.lineMaximumDirection.y, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.lineMinimumAmount, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.lineMaximumAmount, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.lineMinimumInterval, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.lineMaximumInterval, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.orbitMinimumRadius, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.orbitMaximumRadius, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.orbitMinimumSpeed, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.orbitMaximumSpeed, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.orbitMinimumAmount, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.orbitMaximumAmount, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.orbitMinimumInterval, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&configuration.orbitMaximumInterval, 4, stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&configuration.tileCount, 4, stream)) return 0;

		//5761BA/571110 publishes the complete configuration before reading any
		//tile. A failed tile preserves that entry and every earlier complete tile.
		waterConfigurations[configuration.id] = configuration;
		for (LONG remaining = configuration.tileCount; remaining > 0; --remaining)
		{
			MapWaterTile* tile = new MapWaterTile;
			tile->configName = name;
			tile->presetName = presetName;
			if (!Resources::g_pResourceSystem->ReadMemory(&tile->position.x, 4, stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&tile->position.y, 4, stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&tile->position.z, 4, stream)
				|| !Common::ReadString(tile->textureName, stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&tile->u, 4, stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&tile->v, 4, stream))
			{
				delete tile; //57638F/56E460, only the incomplete record.
				return 0;
			}
			tile->width = tile->height = 0.125f;
			tile->u = float(double(tile->u) + 0.00390625);
			tile->v = float(double(tile->v) + 0.00390625);
			tile->width = float(double(tile->width) - 0.0078125);
			tile->height = float(double(tile->height) - 0.0078125);
			waterTiles.push_back(tile); //5725D0 is the actual std::list append.
		}
		return 1;
	}

	void Map::ClearWaterTiles()
	{
		//57ADE0..57AE16: delete each record before clearing list nodes.
		for (std::list<MapWaterTile*>::iterator tile = waterTiles.begin(); tile != waterTiles.end(); ++tile)
			delete *tile;
		waterTiles.clear();
	}
}
