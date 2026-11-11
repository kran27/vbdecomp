#include "GameEntity.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace F3
{
	LONG GameEntity::QueueToCrouch()
	{
		LONG result = 0;
		switch (value2C)
		{
		case 1: case 4: break;
		case 2: case 3: case 6: case 7: case 8: case 9:
			result = QueueToDefault();
			value2C = 5;
			result += QueueToCrouch();
			break;
		case 5:
			switch (static_cast<LONG>(weaponStance))
			{
			case 7: result = QueueAnimationPercent(424, 0, 1, 0, 0, 0); break;
			case 1: case 2: case 3: case 4: case 5: case 6:
				result = QueueUnequip();
				result += QueueAnimationPercent(424, 0, 1, 0, 0, 0);
				break;
			default:
				JE::LogDebugString("GameEntity::Queue_To_Crouch() - Unknown weapon stance encountered.\n");
				break;
			}
			break;
		default:
			JE::LogDebugString("GameEntity::Queue_To_Crouch() - Unexpected RootAnimState encountered.\n");
			break;
		}
		return result;
	}
}
