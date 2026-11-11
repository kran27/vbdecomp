#include "AttackMode.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <stddef.h>
#include <string.h>
#include <stdexcept>

namespace F3
{

	AttackMode& AttackMode::operator=(const AttackMode& source)
	{
		if (this != &source)
		{
			// Only the proprietary scalar prefix/tail is copied as bytes. The real
			// compiler's string has ordinary lifetime and assignment, never an overlay.
			memcpy(&kind, &source.kind, offsetof(AttackMode, effectName));
			effectName = source.effectName;
			memcpy(&flag64, &source.flag64, 3);
		}
		return *this;
	}

	void AttackMode::MakeFailsafe()
	{
		kind = 3; animation = 27; damageType = 4; value0C = 0;
		DWORD value = 1; memcpy(unwritten10, &value, 4);
		shots = 0; values18[0] = values18[1] = 0; values18[2] = 3;
		range = 3; values28[0] = 0; values28[1] = 0;
		minimumDamage = 1; maximumDamage = 2;
		flag38 = 1; calledShot = 0; thrown = 0;
		actionPoints = 3; pose = 97; nameRef = 888;
		effectName.clear(); flag64 = 1; flag65 = 0; flag66 = 0;
		// Original failsafe leaves the consumed byte unwritten.
	}

	BYTE AttackMode::Read(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream)
	{
		return ReadData(chunk, stream, 0, 0);
	}

	BYTE AttackMode::ReadWeapon(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		DWORD weaponClass)
	{
		return ReadData(chunk, stream, 1, weaponClass);
	}

	BYTE AttackMode::ReadData(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE weapon, DWORD weaponClass)
	{
		if (!JE::Common::ReadMemory(&animation, 4, stream) || !JE::Common::ReadMemory(&damageType, 4, stream)
			|| !JE::Common::ReadMemory(&shots, 4, stream)) return 0;
		for (DWORD i = 0; i < 3; ++i) if (!JE::Common::ReadMemory(values18 + i, 4, stream)) return 0;
		if (!JE::Common::ReadMemory(&range, 4, stream)) return 0;
		for (DWORD i = 0; i < 2; ++i) if (!JE::Common::ReadMemory(values28 + i, 4, stream)) return 0;
		BYTE value;
		if (!JE::Common::ReadMemory(&minimumDamage, 4, stream)
			|| !JE::Common::ReadMemory(&maximumDamage, 4, stream)
			|| !JE::Common::ReadMemory(&value, 1, stream)) return 0;
		flag38 = value != 0;
		if (chunk.version)
		{
			if (!JE::Common::ReadMemory(&value0C, 4, stream) || !JE::Common::ReadMemory(&value, 1, stream)) return 0;
			calledShot = value != 0;
			if (!JE::Common::ReadMemory(&actionPoints, 4, stream)
				|| !JE::Common::ReadMemory(&value, 1, stream)) return 0;
			consumed = value != 0;
			if (!JE::Common::ReadMemory(&value, 1, stream)) return 0;
			thrown = value != 0;
			if (thrown && consumed)
				JE::FatalError("GameWeapon::ReadAttackModeData() - A weapon can't be both consumed and thrown.");
		}
		if (chunk.version >= 2 && !JE::Common::ReadMemory(&pose, 4, stream)) return 0;
		if (chunk.version >= 3 && !JE::Common::ReadMemory(&nameRef, 4, stream)) return 0;
		if (chunk.version >= 4 && !JE::Common::ReadString(effectName, stream)) return 0;
		if (chunk.version >= 5 && (!JE::Common::ReadMemory(&flag64, 1, stream)
			|| !JE::Common::ReadMemory(&flag65, 1, stream))) return 0;
		//Versions below2 continue with the existing unwritten pose, as in both readers.
		if (weapon)
		{
			if (thrown) kind = 2;
			else if (weaponClass >= 1 && weaponClass <= 5) kind = 1;
			else if (weaponClass >= 6 && weaponClass <= 8) kind = 0;
			else if (weaponClass == 9 || weaponClass == 10) kind = 3;
			// Other classes leave kind unwritten in the actual4148A0 stack record.
		}
		if (!pose)
		{
			switch (animation)
			{
			case 1: pose = weapon && flag66 ? 62 : 56; break;
			case 2: pose = 59; break;
			case 4: pose = 61; break;
			case 6: pose = 60; break;
			case 7: pose = 63; break;
			case 8: pose = 64; break;
			case 9: pose = 65; break;
			case 10: case 11: case 12: pose = animation + 43; break;
			default: if (animation >= 13 && animation <= 34) pose = animation + 70; break;
			}
		}
		return 1;
	}
}
