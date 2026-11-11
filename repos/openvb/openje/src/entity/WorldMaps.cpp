#include "World.hpp"
#include "Map.hpp"
#include "WorldCollections.hpp"
#include "EntityEnvironment.hpp"
#include "model/ModelServices.hpp"
#include "resources/ResourceSystem.hpp"
#include <cstring>
#include <new>

namespace JE
{
	WorldMap* World::GetWorldMap(const char* name) //5477E0, IWorld+8C.
	{
		for (std::list<WorldMap*>::iterator i=worldMapResources.begin();i!=worldMapResources.end();++i)
		{
			WorldMap* resource = *i;
			if (resource && !_stricmp(name, resource->resourceName.c_str())) return resource;
		}
		if (!Resources::g_pResourceSystem->Contains(name)) return NULL;
		WorldMap* resource=NewWorldMap();
		resource->Read(name); //Publication follows Read, unlike the ordinary Map loader.
		worldMapResources.insert(worldMapResources.begin(), resource);
		return resource;
	}

	DWORD World::AllocateMapID() { return (++value84 & 0x1FFFFFFF) | 0x20000000; }
	Map* World::NewMap() { return new Map; }
	Map* World::GetMap(DWORD id)
	{
		for (WorldMaps::iterator i = maps.begin(); i != maps.end(); ++i)
		{
			Map* map = i->second;
			if (map->idB4 == id) return map;
		}
		return NULL;
	}
	OCTREE* World::GetCurrentMapOctree()
	{
		Map* map = GetCurrentMap(); //544CF0 calls virtual IWorld+60, preserving overrides.
		OCTREE* result = NULL;
		if (map) result = map->GetOctree();
		return result;
	}
	Map* World::LoadMap(const char* name)
	{
		char resource[260];
		strcpy(resource, name);
		size_t length = strlen(resource);
		if (length < 4 || _stricmp(resource + length - 4, ".map")) strcat(resource, ".map");
		WorldMaps::iterator found;
		{
			const std::string lookupName(resource);
			found = maps.find(lookupName);
		} //547D46 destroys the lookup key before the result/resource checks.
		if (found != maps.end()) return found->second;
		if (!Resources::g_pResourceSystem->Contains(resource)) return NULL;
		IWorld* world = this;
		Map* map = world->NewMap();
		map->Create(resource);
		map->GetOctree();
		{
			const std::string publicationName(resource);
			maps[publicationName] = map;
		} //547DFB publishes before key destruction and InitializeGeometry.
		map->InitializeGeometry();
		return map;
	}
	void World::SetActiveMap(const std::string& name)
	{
		WorldMaps::iterator found = maps.find(name);
		Map* selected = found->second; //546184 dereferences even the end iterator.
		const DWORD previousID = valueE0;
		if (previousID == selected->idB4) return;
		Map* previous = NULL;
		if (previousID)
		{
			IWorld* world = g_World;
			previous = world->GetMap(previousID);
		}
		const DWORD selectedID = selected->idB4; //Reload after the previous-map callback.
		valueE0 = selectedID;
		IWorld* world = g_World;
		Map* current = world->GetMap(selectedID);
		if (previous) previous->SetActiveMap(valueE0);
		if (current) current->SetActiveMap(valueE0);
	}
	void World::SetActiveMap(DWORD id) //545820.
	{
		//The original search result is unused; an unknown ID is still published.
		for (WorldMaps::iterator i=maps.begin();i!=maps.end();++i)
			if (i->second->idB4==id) break;
		const DWORD previousID = valueE0;
		if (previousID == id) return;
		Map* previous = NULL;
		if (previousID)
		{
			IWorld* world = g_World;
			previous = world->GetMap(previousID);
		}
		IWorld* world = g_World;
		Map* current = world->GetMap(id);
		valueE0 = id; //Unlike the name overload, publish AFTER both lookups.
		if (previous) previous->SetActiveMap(id);
		if (current) current->SetActiveMap(valueE0);
	}
	void World::LoadMainmenu(DWORD)
	{
		//5454F0 reads the clock only when leaving state4, not for normal1->3.
		ChangeState(3);
		std::string name("none.map");
		const Configuration::Settings& settings = GetEntityEnvironment().EngineSettings();
		Configuration::Settings::const_iterator section = settings.find("Mainmenu");
		if (section != settings.end())
		{
			Configuration::Values::const_iterator value = section->second.find("Map Name");
			if (value != section->second.end()) name = value->second;
		}
		if (Resources::g_pResourceSystem->Contains(name.c_str()))
		{
			IWorld* world = this; //547175 captures this receiver, not the singleton.
			world->LoadMap(name.c_str());
			IWorldClient* client = this;
			client->SetActiveMap(name);
		}
	}
}
