#include <winsock2.h>
#include <cstdlib>
#include <new>
#include "World.hpp"
#include "Entity.hpp"
#include "startup/NetworkComponent.hpp"
#include "startup/MessageLoop.hpp"
#include "network/EngineClient.hpp"
#include "graphics/Graphics.hpp"
#include "Map.hpp"
#include "PathManager.hpp"
#include "Combat.hpp"
#include "Gregorian.hpp"
#include "WorldCollections.hpp"
#include "model/ModelInstance.hpp"
#include "graphics/MapMaskManager.hpp"

namespace JE
{
	namespace
	{
		void DetachEntityModels(Entity* entity, void*)
		{
			if (entity->model) entity->model->DetachAll(); //546933 ->564460.
		}
	}

	void World::BeforeCleanup() {} //Shared404B70, also the original GameWorld hook.

	void World::ClearAuxiliaryObjects() //5460D0; Game46FB10 is a direct thunk.
	{
		//5477E0 establishes the former unknown+2C payload as actual WorldMap.
		//5460D0 deletes each payload before clearing any of the owning list nodes.
		for (std::list<WorldMap*>::iterator entry=worldMapResources.begin();entry!=worldMapResources.end();++entry)
			if (*entry) delete *entry;
		worldMapResources.clear();
	}

	void World::Cleanup() //5455B0.
	{
		BeforeCleanup();
		Reset();
		if (StoreManager* owner = stores)
		{
			owner->StoreManager::~StoreManager();
			::operator delete(owner);
		}
		if (Encyclopedia* owner = encyclopedia)
		{
			owner->Encyclopedia::~Encyclopedia();
			::operator delete(owner);
		}
		if (journal) delete journal;
		if (Party* owner = party)
		{
			owner->Party::~Party();
			::operator delete(owner);
		}
		if (PathManager* owner = paths)
		{
			owner->PathManager::~PathManager();
			::operator delete(owner);
		}
		if (calendar) ::operator delete(calendar);
		if (combat) delete combat;
		if (Cutscene* owner = cutscene154)
		{
			owner->Cutscene::~Cutscene();
			::operator delete(owner);
		}
		if (lightIndices)
		{
			free(lightIndices);
			lightIndices = NULL;
		}
	}

	void World::Reset() //546870, original receiver is the IWorld subobject.
	{
		WorldMaps::iterator entry = maps.begin();
		while (entry != maps.end())
		{
			if (entry->second) delete entry->second;
			//The map stays published through deletion; erase returns its successor.
			entry = maps.erase(entry);
		}
		if (F3::g_pNetworkInstance->GetMode() & 1)
			combat->Cleanup();
		journal->Cleanup();
		encyclopedia->Initialize();
		stores->Cleanup();
		ClearAuxiliaryObjects();
		static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x74->maskManager->Clear();
		party->Cleanup();
		for (std::list<Entity*>::iterator entity = g_createdEntities.begin();
			entity != g_createdEntities.end(); ++entity)
		{
			if ((*entity)->model) (*entity)->model->DetachAll();
		}
		while (g_createdEntities.begin() != g_createdEntities.end())
			delete *g_createdEntities.begin();

		time48 = 0;
		value4C = F3::GetElapsedTimeMS();
		value50 = 0;
		value54 = F3::GetElapsedTimeMS();
		value68 = 0;
		value6C = 0;
		value70 = 0;
		calendar->SetDate(initialDate); //5469BF borrows the actual live date; required owner.
		value7C = 0; //5469C7 is the real producer for the ctor-unwritten effect counter.
		value80 = 0;
		value84 = 0;
		value88 = 0;
		valueE0 = 0;
		parameters90.Load("", "globals.ini");
		parametersB8.Load("", "locals.ini");
		visibleObjects.clear(); //Supplied VC7 clear frees storage before resetting its pointers.
		visibleLights.clear();
		lateEffects.clear();
		//Original does not change value38 or clear the owner pointers here.
	}
}
