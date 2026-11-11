#include "GameWorld.hpp"
#include "GameAmmo.hpp"
#include "GameActionIdle.hpp"
#include "GameActionDie.hpp"
#include "GameActionMoveTo.hpp"
#include "GameActionPickUp.hpp"
#include "GameActionWait.hpp"
#include "GameActionWander.hpp"
#include "GameActionFloatingSpeechText.hpp"
#include "GameEquipmentActions.hpp"
#include "GamePathManager.hpp"
#include "GameWorldCollections.hpp"
#include "GameMap.hpp"
#include "EntityTypes.hpp"
#include "entity/EntityResourceTypes.hpp"
#include "entity/EntityEnvironment.hpp"
#include <string.h>

namespace F3
{
	GameEncyclopedia::GameEncyclopedia() {}
	GameStoreManager::GameStoreManager() {}
	GameWorldMap::GameWorldMap() {}

	GameEncyclopedia::~GameEncyclopedia() {}
	GameStoreManager::~GameStoreManager() {}
	GameWorldMap::~GameWorldMap() {}

	namespace
	{
		// Original6F7E10..6F803F: twenty real28-byte VC7 strings, in this order.
		std::string ammunitionResources[] = {
			"Ammo12_ga.AMO", "Ammo15mm_EC.AMO", "Ammo223.AMO", "Ammo22.AMO",
			"Ammo22_Injector.AMO", "Ammo2mm_EC.AMO", "Ammo30.AMO", "Ammo40mm_Rifle_Grenade.AMO",
			"Ammo44.AMO", "Ammo45.AMO", "Ammo50.AMO", "Ammo762mm.AMO", "Ammo9mm.AMO",
			"AmmoBB.AMO", "AmmoMicro_Fusion_Cell.AMO", "AmmoNaphate.AMO", "AmmoHeavy_Rivet.AMO",
			"AmmoLil_Rivet.AMO", "AmmoRocket.AMO", "AmmoSmall_Energy_Cell.AMO"
		};
	}

	GameWorld::GameWorld() { InitializeAmmunition(); }
	void GameWorld::ClearAuxiliaryObjects() //46FB10, actual5-byte base tail call.
	{
		World::ClearAuxiliaryObjects();
	}
	GameWorld::~GameWorld()
	{
		JE::GetEntityEnvironment().BeforeWorldDestruction();
	}
	JE::Entity* GameWorld::NewEntity(DWORD type)
	{
		if (type > 10) return World::NewEntity(type);
		return CreateGameEntity(type);
	}
	JE::Action* GameWorld::NewAction(DWORD type)
	{
		if (type == 0) return new GameActionDie;
		if (type == 1) return new GameActionIdle;
		if (type == 2) return new GameActionMoveTo;
		if (type == 3) return new GameActionPickUp;
		if (type == 5) return new GameActionWait;
		if (type == 4) return new GameActionWander;
		if (type == 8) return new GameActionFloatingSpeechText;
		switch (type)
		{
		case 9: return new GameActionEquip;
		case 10: return new GameActionUnequip;
		case 11: return new GameActionSetActiveWeapon;
		default: return World::NewAction(type);
		}
	}
	DWORD GameWorld::ClassifyEntity(const std::string& name, BYTE alternate)
	{
		return ClassifyEntity(name.c_str(), alternate);
	}
	DWORD GameWorld::ClassifyEntity(const char* name, BYTE alternate)
	{
		char extension[260];
		JE::Common::ExtractExtension(extension, name); //Original ignores its return.
		DWORD type;
		if (!_stricmp(extension, ".crt")) type = alternate ? 10 : 1;
		else if (!_stricmp(extension, ".itm")) type = 5;
		else if (!_stricmp(extension, ".wea")) type = 6;
		else if (!_stricmp(extension, ".arm")) type = 7;
		else if (!_stricmp(extension, ".dor")) type = 3;
		else if (!_stricmp(extension, ".use")) type = 2;
		else if (!_stricmp(extension, ".pro")) type = 8;
		else if (!_stricmp(extension, ".amo")) type = 9;
		else if (!_stricmp(extension, ".use")) type = 2; //The original repeats this test.
		else if (!_stricmp(extension, ".con")) type = 4;
		else type = JE::World::ClassifyEntity(name, alternate);
		return type;
	}

	JE::Map* GameWorld::NewMap() { return new GameMap; }
	JE::PathManager* GameWorld::NewPathManager() { return new GamePathManager; }
	JE::Party* GameWorld::NewParty() { return new GameParty; }
	JE::Journal* GameWorld::NewJournal() { return new GameJournal; }
	JE::Encyclopedia* GameWorld::NewEncyclopedia() { return new GameEncyclopedia; }
	JE::StoreManager* GameWorld::NewStoreManager() { return new GameStoreManager; }
	JE::WorldMap* GameWorld::NewWorldMap() { return new GameWorldMap; }
	GameAmmo* GameWorld::FindAmmunition(LONG caliber, LONG modifier)
	{
		if (caliber > 20 || caliber < 0 || modifier > 7 || modifier < 0) return NULL;
		// Original accepts caliber20 too; retain its unchecked out-of-array domain.
		return dynamic_cast<GameAmmo*>(JE::GetEntityPtr(ammoIDs[7 * caliber + modifier]));
	}
	BYTE GameWorld::InitializeAmmunition()
	{
		memset(ammoIDs, 0, sizeof(ammoIDs));
		for (std::string* resource = ammunitionResources;
			resource < ammunitionResources + sizeof(ammunitionResources)/sizeof(ammunitionResources[0]); ++resource)
		{
			GameAmmo* ammo = new GameAmmo;
			ammo->Create(resource->c_str(), 0, 0);
			ammoIDs[ammo->modifier + 7 * ammo->caliber] = ammo->entityID;
		}
		return 1;
	}
}
