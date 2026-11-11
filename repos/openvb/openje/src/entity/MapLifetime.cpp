#include "Map.hpp"
#include "MapEffect.hpp"
#include "MapFog.hpp"
#include "graphics/Octree.hpp"
#include "scripting/ScriptSmall.hpp"
#include "Water.hpp"
#include "graphics/Water.hpp"
#include "graphics/WaterSampling.hpp"

namespace JE
{
	void Map::OnSecondUpdate(DWORD)
	{
		ChangeState(3);
	}

	Map::~Map()
	{
		//57AC70..57ACCD holds the next list node, unlinking the current one
		//before SetMap and virtual Entity destruction. The Map remains registered
		//in World throughout this destructor; World reset erases it afterwards.
		std::list<Entity*>::iterator entity = activeEntities.begin();
		while (entity != activeEntities.end())
		{
			Entity* value = *entity;
			std::list<Entity*>::iterator current = entity++;
			activeEntities.erase(current);
			value->SetMap(0);
			if (!value->visibilityOwnerFlag) delete value;
		}
		if (script) delete script;
		if (terrainGrid) delete terrainGrid;
		if (entityGrid) delete entityGrid;
		if (alternateTerrain) delete alternateTerrain; //57AD13..57AD25, same real RLEGrid child.
		if (areaMap && ownsAreaMap) delete areaMap;
		ClearWaterObjects(); //56E7E0 unlinks before invoking each virtual destructor.
		ClearTriggers();
		ClearEntryPoints();
		ClearNavigation();
		ClearWaypoints();
		ClearSounds();
		for (std::map<std::string,MapEffect*>::iterator effect = mapEffects.begin(); effect != mapEffects.end(); ++effect)
			delete effect->second; //572560 deletes values before clearing map nodes.
		mapEffects.clear();
		for (std::list<Gfx_Water*>::iterator water=waterSampleOwners.begin();water!=waterSampleOwners.end();++water)
			RemoveWaterSampleOwners((*water)->registryName); //57ADB0..57ADB5.
		ClearWaterTiles();
		ClearVisualEffects(); //56F230: preserve independent delete/release flags, including flag0 leaks.
		if (octree) octree->Release();
		//Automatic real-container teardown owns only list/map nodes. In particular,
		//the pending Entity* list is not walked or deleted by the original.
	}
	void Map::ClearWaterObjects()
	{
		for (std::list<Water*>::iterator i=waterObjects.begin(); i!=waterObjects.end();)
		{
			Water* water=*i;
			i=waterObjects.erase(i);
			delete water;
		}
	}
}
