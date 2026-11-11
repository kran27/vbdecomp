#include "GameEntity.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace F3
{
	LONG GameEntity::QueueEquip() //465AE0.
	{
		LONG result = 0;
		switch (value2C)
		{
		case 0: case 1: case 4: case 6: case 7: case 8: case 9: break;
		case 3:
			if (weaponStance != 7) result = QueueAnimationPercent(314,0,1,0,0,0);
			else result = QueueAnimationPercent(341,0,1,0,0,0);
			break;
		case 2: case 5:
			switch (weaponStance)
			{
			case 1: result = QueueAnimationPercent(23,0,1,0,0,0); break;
			case 2: result = QueueAnimationPercent(73,0,1,0,0,0); break;
			case 3: result = QueueAnimationPercent(104,0,1,0,0,0); break;
			case 4: result = QueueAnimationPercent(135,0,1,0,0,0); break;
			case 5: result = QueueAnimationPercent(167,0,1,0,0,0); break;
			case 6: result = QueueAnimationPercent(203,0,1,0,0,0); break;
			case 7: result = QueueAnimationPercent(250,0,1,0,0,0); break;
			default:
				JE::LogDebugString("GameEntity::Queue_Equip() - Unsupported weapon stance encountered.\n");
				break;
			}
			break;
		default:
			JE::LogDebugString("GameEntity::Queue_Equip() - Unsupported rootanimstate found.\n");
			break;
		}
		return result;
	}

	LONG GameEntity::QueueUnequip() //4636B0.
	{
		LONG result = 0;
		switch (value2C)
		{
		case 0: case 1: case 4: case 6: case 7: case 8: case 9: break;
		case 3:
			if (weaponStance != 7) result = QueueAnimationPercent(330,0,1,0,0,0);
			else result = QueueAnimationPercent(352,0,1,0,0,0);
			break;
		case 2: case 5:
			switch (weaponStance)
			{
			case 1: result = QueueAnimationPercent(60,0,1,0,0,0); break;
			case 2: result = QueueAnimationPercent(91,0,1,0,0,0); break;
			case 3: result = QueueAnimationPercent(123,0,1,0,0,0); break;
			case 4: result = QueueAnimationPercent(154,0,1,0,0,0); break;
			case 5: result = QueueAnimationPercent(190,0,1,0,0,0); break;
			case 6: result = QueueAnimationPercent(221,0,1,0,0,0); break;
			case 7: result = QueueAnimationPercent(296,0,1,0,0,0); break;
			default:
				JE::LogDebugString("GameEntity::Queue_Unequip() - Unsupported weapon stance encountered.\n");
				break;
			}
			break;
		default:
			JE::LogDebugString("GameEntity::Queue_Unequip() - Unsupported rootanimstate found.\n");
			break;
		}
		return result;
	}
}
