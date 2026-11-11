#include "network/EngineServer.hpp"
#include "World.hpp"
#include "EntityEnvironment.hpp"
#include "Gregorian.hpp"
#include "Combat.hpp"
#include "PathManager.hpp"
#include "WorldCollections.hpp"
#include "Map.hpp"
#include "configuration/ResourceConfigurationRead.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "startup/Path.hpp"
#include "startup/EngineSettings.hpp"
#include "startup/MessageLoop.hpp"
#include "startup/NetworkComponent.hpp"
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>

namespace JE
{
	void WorldParameters::Load(std::string directory, std::string filename)
	{
		if (filename.empty())
			name = "globals.ini";
		else
			name = filename;
		if (directory.empty())
		{
			if (!Configuration::ReadResourceConfigurationFile(name, values))
				FatalError("Unable to load var file %s from database.\n", name.c_str());
			return;
		}
		char path[260];
		strcpy(path, directory.c_str());
		F3::PathConcat(path, name.c_str());
		if (!Configuration::ReadFile(path, values))
		{
			FatalError("Unable to load var file %s from %s.\n", name.c_str(), path);
			return;
		}
	}

	void World::ReadSettings()
	{
		value40 = 6;
		Configuration::Settings& settings = *F3::GetEngineSettings();
		Configuration::Settings::iterator section = settings.find("World");
		if (section == settings.end()) return;
		Configuration::Values& entries = section->second;
		{
			Configuration::Values::const_iterator value = entries.find("Seconds Per Round");
			if (value != entries.end()) value40 = std::atol(value->second.c_str());
		}
		{
			Configuration::Values::const_iterator value = entries.find("Year");
			if (value != entries.end()) initialDate.year = std::atol(value->second.c_str());
		}
		{
			Configuration::Values::const_iterator value = entries.find("Month");
			if (value != entries.end()) initialDate.month = std::atol(value->second.c_str());
		}
		{
			Configuration::Values::const_iterator value = entries.find("Day");
			if (value != entries.end()) initialDate.day = std::atol(value->second.c_str());
		}
		{
			Configuration::Values::const_iterator value = entries.find("Hour");
			if (value != entries.end()) initialDate.hour = std::atol(value->second.c_str());
		}
		{
			Configuration::Values::const_iterator value = entries.find("Minute");
			if (value != entries.end()) initialDate.minute = std::atol(value->second.c_str());
		}
		{
			Configuration::Values::const_iterator value = entries.find("Second");
			if (value != entries.end()) initialDate.second = std::atol(value->second.c_str());
		}
	}

	void World::Initialize()
	{
		ReadSettings();
		time48 = 0;
		value4C = F3::GetElapsedTimeMS();
		value50 = 0;
		value54 = F3::GetElapsedTimeMS();
		if (F3::g_pNetworkInstance->GetMode() & 1)
		{
			combat = NewCombat();
			combat->Initialize();
		}
		parameters90.Load("", "globals.ini");
		parametersB8.Load("", "locals.ini");
		calendar = NewCalendar();
		paths = NewPathManager();
		party = NewParty();
		journal = NewJournal();
		encyclopedia = NewEncyclopedia();
		stores = NewStoreManager();
		cutscene154 = NewCutscene();
		ChangeState(1);
	}

	Combat* World::NewCombat() { return new Combat; }
	Gregorian* World::NewCalendar()
	{
		return new Gregorian(initialDate);
	}
	PathManager* World::NewPathManager() { return new PathManager; }
	Party* World::NewParty() { return new Party; }
	Journal* World::NewJournal() { return new Journal; }
	Encyclopedia* World::NewEncyclopedia() { return new Encyclopedia; }
	StoreManager* World::NewStoreManager() { return new StoreManager; }
	Cutscene* World::NewCutscene() { return new Cutscene; }
	WorldMap* World::NewWorldMap() { return new WorldMap; }

	void World::ReleaseInitializedOwners()
	{
		//Compatibility suffix: preserve native per-owner capture and dangling fields.
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

	void World::UpdateServerEntities(DWORD worldTime)
	{
		const bool updateCutscene = cutscene154->state != 0;
		combat->Update(GetServerTime());
		paths->Update();
		if (combat->active) return;
		for (WorldMaps::iterator i = maps.begin(); i != maps.end(); ++i)
			i->second->Update(GetServerTime());
		UpdateEntities(*this);
		if (!combat->actors.empty() && combat->mode)
		{
			AdvanceRoundServerTime();
			calendar->AddSeconds(value40);
		}
		else
		{
			float elapsed = static_cast<float>(worldTime - value68);
			float scaled = value74 * elapsed;
			AdvanceNormalServerTime(worldTime);
			value44 = static_cast<DWORD>(static_cast<__int64>(double(value44) + scaled));
			if (value44 >= 1000)
			{
				calendar->AddSeconds(value44 / 1000);
				value44 %= 1000;
			}
		}
		if (updateCutscene) cutscene154->Update();
		GetEntityEnvironment().GetServer()->SetScriptStep(0); //545FDD, actual Server5567F0.
	}
}
