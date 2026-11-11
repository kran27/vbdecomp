#include "GameCreature.hpp"
#include "GameArmor.hpp"
#include "GameInventory.hpp"
#include "resources/ResourceSystem.hpp"
#include "entity/World.hpp"
#include "model/ModelInstance.hpp"
#include "graphics/ModelInstance.hpp"
#include "graphics/ModelData.hpp"
#include "graphics/StringFormat.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <string.h>
#include <stdexcept>

namespace F3
{
	std::string GameCreature::GetSocketModel(DWORD slot) const
	{
		return sockets[slot].model;
	}

	std::string GameCreature::GetSocketTexture(DWORD slot) const
	{
		return sockets[slot].texture;
	}

	GameCreature::GameCreature() //40D010.
	{
		entityClass = 1;
		value294 = 10;
		value2B4 = 0;
		materialMaskMode = 0;
		sockets[0].model = modelName;
		sockets[0].texture = textureOverride;
		memset(special, 0, sizeof(special));
		baseValues[0] = 1; baseValues[1] = 0; baseValues[2] = 1;
		baseValues[3] = 0; baseValues[4] = 18; baseValues[5] = 0; baseValues[6] = 0;
		valueD8 = 0; valueDC = 0;
		value7EC = 0;
		value274 = 0;
		memset(skills, 0, sizeof(skills)); memset(traits, 0, sizeof(traits));
		memset(uncalledChances, 0, sizeof(uncalledChances));
		for (DWORD i = 0; i < 6; ++i)
		{
			resistances[i] = 0; baseResistances[i] = 0;
			thresholds[i] = 0; baseThresholds[i] = 0;
		}
		memset(skills, 0, sizeof(skills)); //Original repeats this before the refresh.
		GameCreature::OnRefreshAssociations();
		currentAttackMode = std::list<AttackMode>::iterator();
		attackModeCount = 0;
		value7FC = 0;
	}

	GameCreature::~GameCreature()
	{
		//40D490 destroys the actual member lists/strings in reverse order.
		//Pending entity pointers are borrowed here: only their list nodes die.
	}

	void GameCreature::OnRefreshAssociations() //4097A0, Entity slot+D8.
	{
		GameEntity::OnRefreshAssociations();
		for (DWORD i = 0; i < 7; ++i) baseSpecial[i] = 0;
		memset(runtimeValues77C, 0, sizeof(runtimeValues77C));
		for (DWORD i = 0; i < 6; ++i) thresholds[i] = 0;
		for (DWORD i = 0; i < 6; ++i) baseThresholds[i] = 0;
	}

	JE::Entity* GameCreature::CreateInventoryItem(const std::string& name)
	{
		if (!JE::g_World) throw std::logic_error("GameCreature40B9E0: actual World required");
		DWORD type = JE::g_World->ClassifyEntity(name, 0);
		JE::Entity* item = JE::g_World->NewEntity(type);
		item->Create(name.c_str(), 0, 0);
		return item;
	}

	void GameCreature::OnCreated()
	{
		GameInventory* owner = dynamic_cast<GameInventory*>(inventory);
		while (!pendingInventory.empty())
		{
			owner->Equip(pendingInventory.front(), 7, 1);
			pendingInventory.pop_front(); // Original discards the pending node even after Equip returns0.
		}
	}

	void GameCreature::OnModelLoaded()
	{
		dynamic_cast<GameInventory*>(inventory)->ReapplyEquipment();
		RestoreSavedAppearance();
	}

	BYTE GameCreature::ReadVersion0(JE::Resources::ResourceMemoryStream* stream)
	{
		for (DWORD i = 0; i < 7; ++i)
		{
			if (!JE::Common::ReadMemory(special + i, 4, stream)) return 0;
			baseSpecial[i] = special[i];
		}
		for (DWORD i = 0; i < 7; ++i) if (!JE::Common::ReadMemory(baseValues + i, 4, stream)) return 0;
		if (!JE::Common::ReadMemory(&valueDC, 4, stream)) return 0;
		DWORD count, index, value;
		if (!JE::Common::ReadMemory(&count, 4, stream)) return 0;
		for (DWORD i = 0; i < count; ++i)
		{
			if (!JE::Common::ReadMemory(&index, 4, stream) || !JE::Common::ReadMemory(&value, 4, stream)) return 0;
			skills[index] = value; // Original trusted index, no clamp or invented defaults.
		}
		if (!JE::Common::ReadMemory(&count, 4, stream)) return 0;
		for (DWORD i = 0; i < count; ++i)
		{
			if (!JE::Common::ReadMemory(&index, 4, stream) || !JE::Common::ReadMemory(&value, 4, stream)) return 0;
			traits[index] = value;
		}
		if (!JE::Common::ReadMemory(&count, 4, stream)) return 0;
		for (DWORD i = 0; i < count; ++i)
		{
			if (!JE::Common::ReadMemory(&value, 4, stream)) return 0;
			selectedTraits.push_back(value);
		}
		if (!JE::Common::ReadMemory(&count, 4, stream)) return 0;
		for (DWORD i = 0; i < count; ++i)
		{
			if (!JE::Common::ReadMemory(&value, 4, stream)) return 0;
			tagSkills.push_back(value);
		}
		return JE::Common::ReadMemory(&value274, 4, stream);
	}

	BYTE GameCreature::ReadVersion1(JE::Resources::ResourceMemoryStream* stream)
	{
		if (!JE::Common::ReadString(portrait, stream) || !JE::Common::ReadMemory(&value294, 4, stream)
			|| !JE::Common::ReadString(value298, stream) || !JE::Common::ReadMemory(&value2B4, 4, stream)
			|| !JE::Common::ReadMemory(&materialMaskMode, 1, stream)) return 0;
		for (DWORD i = 0; i < 12; ++i) if (!JE::Common::ReadString(materialNames[i], stream)) return 0;
		for (DWORD i = 1; i < 13; ++i)
			if (!JE::Common::ReadString(sockets[i].model, stream)
				|| !JE::Common::ReadString(sockets[i].texture, stream)) return 0;
		for (DWORD i = 0; i < 12; ++i)
		{
			BYTE value;
			if (!JE::Common::ReadMemory(&value, 1, stream)) return 0;
			socketFlags[i] = value != 0;
		}
		return 1;
	}

	BYTE GameCreature::ReadVersion2(JE::Resources::ResourceMemoryStream* stream)
	{
		if (!JE::Common::ReadMemory(&attackModeCount, 4, stream))
		{
			JE::LogDebugString("GameCreature::ReadVersion2 - No Mode counter found");
			return 0;
		}
		if (!attackModeCount)
		{
			JE::LogDebugString("GameCreature::ReadVersion2() - Making failsafe attack mode for entity: %s\n", resourceName.c_str());
			AttackMode mode;
			mode.MakeFailsafe();
			attackModes.push_back(mode);
		}
		return 1;
	}

	BYTE GameCreature::ReadVersion3(JE::Resources::ResourceMemoryStream* stream)
	{
		for (DWORD i = 0; i < 8; ++i) if (!JE::Common::ReadMemory(uncalledChances + i, 4, stream)) return 0;
		for (DWORD i = 0; i < 6; ++i)
		{
			if (!JE::Common::ReadMemory(baseResistances + i, 4, stream)) return 0;
			resistances[i] = baseResistances[i];
		}
		for (DWORD i = 0; i < 6; ++i)
		{
			if (!JE::Common::ReadMemory(baseThresholds + i, 4, stream)) return 0;
			thresholds[i] = baseThresholds[i];
		}
		return 1;
	}

	BYTE GameCreature::ReadVersion4(JE::Resources::ResourceMemoryStream* stream)
	{
		DWORD count;
		if (!JE::Common::ReadMemory(&count, 4, stream)) return 0;
		for (DWORD i = 0; i < count; ++i)
		{
			std::string item;
			if (!JE::Common::ReadString(item, stream)) return 0;
			JE::Entity* entity = CreateInventoryItem(item);
			pendingInventory.push_back(entity);
		}
		return 1;
	}

	BYTE GameCreature::ReadAttackModeData(JE::Common::Chunk& chunk,
		JE::Resources::ResourceMemoryStream* stream)
	{
		const DWORD version = chunk.version;
		AttackMode mode;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.animation, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.damageType, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.shots, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.values18[0], 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.values18[1], 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.values18[2], 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.range, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.values28[0], 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.values28[1], 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.minimumDamage, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.maximumDamage, 4, stream)) return 0;
		BYTE value;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
		mode.flag38 = value != 0;
		if (version >= 1)
		{
			if (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.value0C, 4, stream)) return 0;
			if (!JE::Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
			mode.calledShot = value != 0;
			if (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.actionPoints, 4, stream)) return 0;
			if (!JE::Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
			mode.consumed = value != 0;
			if (!JE::Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
			mode.thrown = value != 0;
			if (mode.thrown && mode.consumed)
				JE::FatalError("GameWeapon::ReadAttackModeData() - A weapon can't be both consumed and thrown.");
		}
		if (chunk.version >= 2 && !JE::Resources::g_pResourceSystem->ReadMemory(&mode.pose, 4, stream)) return 0;
		if (chunk.version >= 3 && !JE::Resources::g_pResourceSystem->ReadMemory(&mode.nameRef, 4, stream)) return 0;
		if (chunk.version >= 4 && !JE::Common::ReadString(mode.effectName, stream)) return 0;
		if (chunk.version >= 5 && (!JE::Resources::g_pResourceSystem->ReadMemory(&mode.flag64, 1, stream)
			|| !JE::Resources::g_pResourceSystem->ReadMemory(&mode.flag65, 1, stream))) return 0;
		if (!mode.pose)
		{
			switch (mode.animation)
			{
			case 1: mode.pose = 56; break;
			case 2: mode.pose = 59; break;
			case 4: mode.pose = 61; break;
			case 6: mode.pose = 60; break;
			case 7: mode.pose = 63; break;
			case 8: mode.pose = 64; break;
			case 9: mode.pose = 65; break;
			case 10: mode.pose = 53; break;
			case 11: mode.pose = 54; break;
			case 12: mode.pose = 55; break;
			case 13: mode.pose = 83; break;
			case 14: mode.pose = 84; break;
			case 15: mode.pose = 85; break;
			case 16: mode.pose = 86; break;
			case 17: mode.pose = 87; break;
			case 18: mode.pose = 88; break;
			case 19: mode.pose = 89; break;
			case 20: mode.pose = 90; break;
			case 21: mode.pose = 91; break;
			case 22: mode.pose = 92; break;
			case 23: mode.pose = 93; break;
			case 24: mode.pose = 94; break;
			case 25: mode.pose = 95; break;
			case 26: mode.pose = 96; break;
			case 27: mode.pose = 97; break;
			case 28: mode.pose = 98; break;
			case 29: mode.pose = 99; break;
			case 30: mode.pose = 100; break;
			case 31: mode.pose = 101; break;
			case 32: mode.pose = 102; break;
			case 33: mode.pose = 103; break;
			case 34: mode.pose = 104; break;
			}
		}
		attackModes.push_back(mode);
		return 1;
	}

	BYTE GameCreature::ReadCreature(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE)
	{
		const DWORD version = chunk.version;
		if (!ReadVersion0(stream) || (version >= 1 && !ReadVersion1(stream))
			|| (version >= 2 && !ReadVersion2(stream))) return 0;
		if (version >= 3)
		{
			if (!ReadVersion3(stream)) return 0;
			DWORD sum = 0;
			for (DWORD i = 0; i < 8; ++i) sum += uncalledChances[i];
			if (sum != 100)
				JE::LogDebugString("SANITY ERROR: Entity %d's baseUncalledChances don't add up to exactly 100\n", entityID);
		}
		if ((version >= 4 && !ReadVersion4(stream))
			|| (version >= 5 && !JE::Resources::g_pResourceSystem->ReadMemory(&valueD8, 4, stream))) return 0;
		for (DWORD i = 0; i < attackModeCount; ++i)
		{
			JE::Common::Chunk attack;
			if (!attack.Read(stream)) return 0;
			if (attack.type != 'MAWG')
			{
				JE::LogDebugString("GameCreature::ReadData() - Unexpected chunk ID. Attack Mode ID 'GWAM' expected.");
				return 0;
			}
			if (!ReadAttackModeData(attack, stream)) return 0;
			attack.SkipToEnd();
		}
		RebuildAttackModes();
		return 1;
	}

	void GameCreature::RebuildAttackModes()
	{
		activeAttackModes.clear();
		for (std::list<AttackMode>::const_iterator i = attackModes.begin(); i != attackModes.end(); ++i)
		{
			activeAttackModes.push_back(*i);
			BYTE addCalledShot = 0;
			switch (i->animation)
			{
			case 2: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
			case 14: case 15: case 16: case 17: case 18: case 19: case 20: case 21:
			case 23: case 25: case 26: case 27: case 30: case 31: case 32: case 34:
				addCalledShot = 1; break;
			}
			if (addCalledShot && !i->calledShot)
			{
				AttackMode copy(*i);
				copy.calledShot = 1;
				copy.actionPoints += 3;
				activeAttackModes.push_back(copy);
			}
		}
		currentAttackMode = activeAttackModes.begin();
	}

	void GameCreature::RecalculateArmor()
	{
		DWORD sums[2][6];
		memset(sums, 0, sizeof(sums));
		if (inventory)
		{
			GameInventory* owner = dynamic_cast<GameInventory*>(inventory);
			std::list<DWORD> slots;
			for (std::list<JE::Entity*>::const_iterator i = owner->items.begin(); i != owner->items.end(); ++i)
			{
				GameArmor* armor = dynamic_cast<GameArmor*>(*i);
				if (!armor || !owner->GetEquippedSlots(*i, slots)) continue;
				while (!slots.empty())
				{
					DWORD slot = slots.front(); slots.pop_front();
					if (slot <= 1)
						for (DWORD value = 0; value < 6; ++value) sums[slot][value] += armor->values944[value];
				}
			}
		}
		for (DWORD i = 0; i < 6; ++i)
		{
			resistances[i] = LONG(DWORD(baseResistances[i]) + sums[0][i]);
			thresholds[i] = LONG(DWORD(baseThresholds[i]) + sums[1][i]);
		}
	}

	BYTE GameCreature::ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE skipDefaultOverride)
	{
		if (chunk.type != 'ERCG') return GameEntity::ReadFromFile(chunk, stream, skipDefaultOverride);
		if (ReadCreature(chunk, stream, skipDefaultOverride)) return 1;
		JE::LogDebugString("Error reading creature game data from entity file.");
		JE::Resources::g_pResourceSystem->CloseMemoryStream(stream);
		return 0;
	}

	void GameCreature::RestoreSavedAppearance()
	{
		if (materialMaskMode)
		{
			JE::ModelInstance* maskedModel = model;
			if (maskedModel)
			{
				Gfx_ModelInstance* base = maskedModel->instances[0];
				if (base)
				{
					const DWORD count = base->data->MaterialCount();
					for (DWORD i = 0; i < count; ++i) base->EnableMaterialByIndex(i, 0);
					const char* names[] = {"Chest","Feet","Hands","Head","Legs","UpperArms",
						"Waist","ForeArms","Shins"};
					for (DWORD i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
						base->EnableMaterial(names[i], 1);
					for (DWORD i = 0; i < 12; ++i)
						if (!materialNames[i].empty()) base->EnableMaterial(materialNames[i], 1);
				}
			}
		}
		JE::ModelInstance* savedModel = model;
		if (!savedModel) return;
		for (DWORD i = 1; i < 13; ++i)
		{
			if (sockets[i].model.empty()) continue;
			std::string normalized = sockets[i].model;
			JE::UpperString(normalized);
			DWORD variation;
			if (savedModel->FindSocketModel(i, normalized, &variation))
				savedModel->SetSocket(i, variation, sockets[i].texture.c_str());
		}
	}
}
