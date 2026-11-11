#include "startup/NetworkComponent.hpp"
#include "GameEntity.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "entity/Inventory.hpp"
#include "GameInventory.hpp"
#include "GameWeapon.hpp"
#include "entity/World.hpp"
#include "entity/Effect.hpp"
#include "resources/ResourceSystem.hpp"
#include <string.h>
#include <stdexcept>

namespace F3
{
	GameEntity::GameEntity()
	{
		for (DWORD i = 0; i < 6; ++i) stringRefs[i] = 0;
		stringRefs[4] = 4;
		maximumHealth = 1;
		currentHealth = 0;
		value90 = 0;
		value94 = 10;
		activeWeaponSlot = 2;
		value8C = DWORD(-1);
		SetUpdateInterval(5); // Actual46797E; distinct from subclass type+15C.
		actionBudget = 0; //467989: vbdisp+178 is Entity+174, not its enabled byte.
		if (g_pNetworkInstance->GetMode() & 1)
			creationServerTime = JE::GetWorldServerInstance()->GetServerTime();
		else creationServerTime = 0;
		value2C = 0;
		weaponStance = 7;
		value34 = 7;
		flag9C = 0;
		pendingAnimation = 1; //4679D9: vbdisp+1C0 is Entity+1BC.
	}

	std::list<JE::Effect*>* GameEntity::GetEffects(DWORD event)
	{
		return &effects[event];
	}

	JE::Inventory* GameEntity::CreateInventory() { return new GameInventory; }
	GameEntity::~GameEntity()
	{
		for (DWORD i = 0; i < 5; ++i)
			while (!effects[i].empty())
			{
				JE::Effect* effect=effects[i].front();
				effects[i].pop_front(); //467B82 unlinks before55A570.
				effect->Destroy();
			}
	}

	BYTE GameEntity::SeparateTextures() const
	{
		if (entityClass != 1) return 0;
		char prefix[8];
		strncpy(prefix, modelName.c_str(), 4);
		prefix[4] = 0;
		return !_stricmp(prefix, "HumM") || !_stricmp(prefix, "HumF");
	}

	void GameEntity::SetMap(DWORD map)
	{
		JE::Entity::SetMap(map);
		JE::Inventory* const owner = inventory;
		for (std::list<JE::Entity*>::iterator i = owner->items.begin(); i != owner->items.end(); ++i)
			if (*i) (*i)->SetMap(map);
	}

	DWORD GameEntity::GetActiveWeaponSlot() const { return activeWeaponSlot; }
	void GameEntity::SetActiveWeaponSlot(DWORD slot) { activeWeaponSlot = slot; }

	void GameEntity::SetWeaponStance(GameWeapon* weapon)
	{
		if (!weapon)
		{
			if (!inventory) return;
			GameInventory* owner = dynamic_cast<GameInventory*>(inventory);
			weapon = dynamic_cast<GameWeapon*>(owner->GetItem(activeWeaponSlot));
		}
		if (!weapon) { weaponStance = 7; return; }
		switch (weapon->weaponClass)
		{
		case 1: case 6: weaponStance = 1; break;
		case 2: case 3: weaponStance = 5; break;
		case 4: weaponStance = 3; break;
		case 5: weaponStance = 6; break;
		case 7: weaponStance = 4; break;
		case 8: weaponStance = 2; break;
		case 9: case 10: weaponStance = 7; break;
		default:
			JE::LogDebugString("GameEntity::SetWeaponStance() - Unknown Weapon Type encountered.\n");
			weaponStance = 7;
		}
	}

	BYTE GameEntity::ReadVersion0(JE::Resources::ResourceMemoryStream* stream)
	{
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&stringRefs[0], 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&stringRefs[1], 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&stringRefs[2], 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&stringRefs[3], 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&stringRefs[4], 4, stream)) return 0;
		return JE::Resources::g_pResourceSystem->ReadMemory(&stringRefs[5], 4, stream) != 0;
	}

	BYTE GameEntity::ReadVersion1(JE::Resources::ResourceMemoryStream* stream)
	{
		LONG& maximum = maximumHealth;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&maximum, 4, stream)) return 0;
		LONG& health = currentHealth;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&health, 4, stream)) return 0;
		if (health > maximum) health = maximum;
		return 1;
	}

	BYTE GameEntity::ReadVersion2(JE::Resources::ResourceMemoryStream* stream) { return 1; }
	BYTE GameEntity::ReadVersion3(JE::Resources::ResourceMemoryStream* stream) { return 1; }

	BYTE GameEntity::ReadGameEntity(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE skipDefaultOverride)
	{
		const DWORD version = chunk.version;
		return ReadVersion0(stream) && (version < 1 || ReadVersion1(stream))
			&& (version < 2 || ReadVersion2(stream)) && (version < 3 || ReadVersion3(stream));
	}

	BYTE GameEntity::ReadEffectTree(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream)
	{
		DWORD count = 0;
		if (chunk.version >= 1 && !JE::Resources::g_pResourceSystem->ReadMemory(&count, 4, stream)) return 0;
		for (DWORD i = 0; i < count; ++i)
		{
			JE::Common::Chunk event;
			if (!event.Read(stream)) return 0;
			if (event.type != 'VEFE')
			{
				JE::LogDebugString("Unexpected effect chunk ID. Event ID 'EFEV' expected.");
				return 0;
			}
			if (!ReadEffectEvent(event, stream)) return 0;
			event.SkipToEnd();
		}
		return 1;
	}

	BYTE GameEntity::ReadEffectEvent(JE::Common::Chunk& chunk,
		JE::Resources::ResourceMemoryStream* stream)
	{
		DWORD key, count = 0;
		if (chunk.version >= 1 && (!JE::Resources::g_pResourceSystem->ReadMemory(&key, 4, stream)
			|| !JE::Resources::g_pResourceSystem->ReadMemory(&count, 4, stream))) return 0;
		for (DWORD record = 0; record < count; ++record)
			if (!ReadEffectRecord(chunk, stream, NULL, key)) return 0;
		return 1;
	}

	BYTE GameEntity::ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE skipDefaultOverride)
	{
		BYTE result;
		const char* error;
		if (chunk.type == 'TNEG')
		{
			result = ReadGameEntity(chunk, stream, skipDefaultOverride);
			error = "Error reading entity game data from entity file.";
		}
		else if (chunk.type == 'RTFE')
		{
			result = ReadEffectTree(chunk, stream);
			error = "Error reading effect tree from entity file.";
		}
		else return JE::Entity::ReadFromFile(chunk, stream, skipDefaultOverride);
		if (!result)
		{
			JE::LogDebugString(error);
			JE::Resources::CloseResourceMemoryStream(stream);
		}
		return result;
	}
}
