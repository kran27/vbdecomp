#include "GameWeapon.hpp"
#include "GameCreature.hpp"
#include "GameWorld.hpp"
#include "resources/ResourceSystem.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <stdexcept>

namespace F3
{
	GameWeapon::GameWeapon() : weaponClass(0), primaryCaliber(0), primaryModifier(0), primaryReloadDuration(2),
		secondaryCaliber(0), secondaryModifier(0), secondaryReloadDuration(2), value1000(1),
		primaryAmmoLoaded(0), primaryAmmoCapacity(1), secondaryAmmoLoaded(0), secondaryAmmoCapacity(1),
		value1020(0), value1024(0), attackModeCount(0), currentAttackMode(),
		primaryAmmo(NULL), secondaryAmmo(NULL)
	{
		entityClass = 6;
		//The actual nullable iterator starts NULL. Read/rebuild publish begin(),
		//which can also retain an allocated empty-list head.
	}

	void GameWeapon::CopyFrom(JE::Entity* source) //415670, virtual adjustor416A60.
	{
		GameWeapon* weapon = dynamic_cast<GameWeapon*>(source);
		if (weapon)
		{
			weaponClass = weapon->weaponClass;
			primaryCaliber = weapon->primaryCaliber;
			primaryReloadDuration = weapon->primaryReloadDuration;
			secondaryCaliber = weapon->secondaryCaliber;
			secondaryReloadDuration = weapon->secondaryReloadDuration;
			value972 = weapon->value972;
			value1000 = weapon->value1000;
			primaryAmmoCapacity = weapon->primaryAmmoCapacity;
			secondaryAmmoCapacity = weapon->secondaryAmmoCapacity;
			value1020 = weapon->value1020;
			value1024 = weapon->value1024;
			attackModes = weapon->attackModes;
			activeAttackModes = weapon->activeAttackModes;
		}
		//Original leaves modifiers, loaded counts, selected mode and ammo owners untouched.
		GameItem::CopyFrom(source);
	}

	GameAmmo* FindAmmunition(DWORD caliber, DWORD modifier) //408110.
	{
		GameWorld* world=dynamic_cast<GameWorld*>(JE::GetWorldInstance());
		if (!world) return NULL;
		return world->FindAmmunition(caliber,modifier);
	}

	GameAmmo* GameWeapon::ResolveAmmunition(DWORD caliber, DWORD modifier)
	{
		GameWorld* world = dynamic_cast<GameWorld*>(JE::g_World);
		return world ? world->FindAmmunition(caliber, modifier) : NULL;
	}

	void GameWeapon::RebuildAttackModes(JE::Entity* owner)
	{
		GameCreature* creature = dynamic_cast<GameCreature*>(owner);
		activeAttackModes.clear();
		for (std::list<AttackMode>::const_iterator i = attackModes.begin(); i != attackModes.end(); ++i)
		{
			activeAttackModes.push_back(*i);
			if (i->animation == 1)
			{
				if (creature && creature->traits[71])
				{
					AttackMode copy(*i);
					copy.flag66 = 1; copy.nameRef = 584; copy.pose = 62;
					activeAttackModes.push_back(copy);
				}
				continue;
			}
			BYTE called = 0;
			switch (i->animation)
			{
			case 2: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
			case 14: case 15: case 16: case 17: case 18: case 19: case 20: case 21:
			case 23: case 25: case 26: case 27: case 30: case 31: case 32: case 34:
				called = 1; break;
			}
			if (called && !i->calledShot)
			{
				AttackMode copy(*i);
				copy.calledShot = 1; copy.actionPoints += 3;
				activeAttackModes.push_back(copy);
			}
		}
		currentAttackMode = activeAttackModes.begin();
	}

	BYTE GameWeapon::ReadVersion0(JE::Resources::ResourceMemoryStream* stream)
	{
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&weaponClass, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&primaryCaliber, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&primaryReloadDuration, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&secondaryCaliber, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&secondaryReloadDuration, 4, stream)) return 0;
		if (!JE::Common::ReadString(value972, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&value1000, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&primaryAmmoLoaded, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&primaryAmmoCapacity, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&secondaryAmmoLoaded, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&secondaryAmmoCapacity, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&value1020, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&value1024, 4, stream)) return 0;
		return JE::Resources::g_pResourceSystem->ReadMemory(&attackModeCount, 4, stream) != 0;
	}

	BYTE GameWeapon::ReadAttackModeData(JE::Common::Chunk& chunk,
		JE::Resources::ResourceMemoryStream* stream)
	{
		const DWORD version = chunk.version;
		AttackMode mode(AttackMode::Weapon);
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
		if (mode.thrown) mode.kind = 2;
		else switch (weaponClass)
		{
		case 1: case 2: case 3: case 4: case 5: mode.kind = 1; break;
		case 6: case 7: case 8: mode.kind = 0; break;
		case 9: case 10: mode.kind = 3; break;
		}
		if (!mode.pose)
		{
			switch (mode.animation)
			{
			case 1: mode.pose = mode.flag66 ? 62 : 56; break;
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

	BYTE GameWeapon::ReadWeapon(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE)
	{
		const DWORD version = chunk.version;
		if (!ReadVersion0(stream)) return 0;
		if (version >= 1 && (!JE::Resources::g_pResourceSystem->ReadMemory(&primaryModifier, 4, stream)
			|| !JE::Resources::g_pResourceSystem->ReadMemory(&secondaryModifier, 4, stream))) return 0;
		for (DWORD i = 0; i < attackModeCount; ++i)
		{
			JE::Common::Chunk attack;
			if (!attack.Read(stream)) return 0;
			if (attack.type != 'MAWG')
			{
				JE::LogDebugString("GameWeapon::ReadData() - Unexpected chunk ID. Attack Mode ID 'GWAM' expected.");
				return 0;
			}
			if (!ReadAttackModeData(attack, stream)) return 0;
			attack.SkipToEnd();
		}
		return 1;
	}

	BYTE GameWeapon::ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE skipDefaultOverride)
	{
		if (chunk.type != 'PWIG') return GameItem::ReadFromFile(chunk, stream, skipDefaultOverride);
		if (!ReadWeapon(chunk, stream, skipDefaultOverride))
		{
			JE::LogDebugString("Error reading weapon game data from item file.\n");
			JE::Resources::g_pResourceSystem->CloseMemoryStream(stream);
			return 0;
		}
		currentAttackMode = attackModes.begin();
		// Actual41559A/4155B4 test capacities at+1008/+1016, not loaded counts.
		if (primaryAmmoCapacity) primaryAmmo = FindAmmunition(primaryCaliber, primaryModifier);
		if (secondaryAmmoCapacity) secondaryAmmo = FindAmmunition(secondaryCaliber, secondaryModifier);
		return 1;
	}
}
