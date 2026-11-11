#include "GameEntity.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace JE
{
	void Entity::ClearQueuedAnimations() //54BD90, actual deque pop-front drain.
	{
		while (!animationQueue.empty()) animationQueue.pop_front();
	}
}

namespace F3
{
	LONG GameEntity::QueueToCombat() //463140.
	{
		LONG result = 0;
		switch (value2C)
		{
		case 1: case 2: break;
		case 3:
			if (static_cast<LONG>(weaponStance) > 0 && static_cast<LONG>(weaponStance) <= 7) result = QueueAnimationPercent(349,0,1,0,0,0);
			else JE::LogDebugString("GameEntity::QueueToCombat() - Unknown Weapon Stance set.\n");
			break;
		case 4:
			result = QueueAnimationPercent(358,0,1,0,0,0);
			value2C = 5;
			result += QueueToCombat();
			break;
		case 5:
			switch (weaponStance)
			{
			case 1: result = QueueAnimationPercent(365,0,1,0,0,0); break;
			case 2: result = QueueAnimationPercent(372,0,1,0,0,0); break;
			case 3: result = QueueAnimationPercent(378,0,1,0,0,0); break;
			case 4: result = QueueAnimationPercent(384,0,1,0,0,0); break;
			case 5: result = QueueAnimationPercent(390,0,1,0,0,0); break;
			case 6: result = QueueAnimationPercent(396,0,1,0,0,0); break;
			case 7: result = QueueAnimationPercent(423,0,1,0,0,0); break;
			default: JE::LogDebugString("GameEntity::QueueToCombat() - Unknown weapon stance set.\n");
			}
			break;
		case 6:
			switch (static_cast<LONG>(weaponStance))
			{
			case 7:
				result = QueueAnimationPercent(446,0,1,0,0,0);
				result += QueueAnimationPercent(327,0,1,0,0,0);
				break;
			case 1: case 2: case 3: case 4: case 5: case 6:
				result = QueueAnimationPercent(439,0,1,0,0,0);
				result += QueueAnimationPercent(327,0,1,0,0,0);
				break;
			default: JE::LogDebugString("GameEntity::QueueToCombat() - Unknown weapon stance is set.\n");
			}
			break;
		case 7:
			switch (weaponStance)
			{
			case 1: result = QueueAnimationPercent(475,0,1,0,0,0); break;
			case 2: result = QueueAnimationPercent(477,0,1,0,0,0); break;
			case 3: result = QueueAnimationPercent(479,0,1,0,0,0); break;
			case 4: result = QueueAnimationPercent(480,0,1,0,0,0); break;
			case 5: result = QueueAnimationPercent(481,0,1,0,0,0); break;
			case 6: result = QueueAnimationPercent(482,0,1,0,0,0); break;
			case 7: result = QueueAnimationPercent(488,0,1,0,0,0); break;
			default: JE::LogDebugString("Entity::QueueToCombat() - Unknown weapon stance set.\n");
			}
			break;
		case 8:
			switch (weaponStance)
			{
			case 1: result = QueueAnimationPercent(491,0,1,0,0,0); break;
			case 2: result = QueueAnimationPercent(492,0,1,0,0,0); break;
			case 3: result = QueueAnimationPercent(494,0,1,0,0,0); break;
			case 4: result = QueueAnimationPercent(495,0,1,0,0,0); break;
			case 5: result = QueueAnimationPercent(496,0,1,0,0,0); break;
			case 6: result = QueueAnimationPercent(497,0,1,0,0,0); break;
			case 7: result = QueueAnimationPercent(503,0,1,0,0,0); break;
			default: JE::LogDebugString("Entity::QueueToCombat() - Unknown weapon stance set.\n");
			}
			break;
		case 9:
			result = QueueAnimationPercent(513,0,1,0,0,0);
			value2C = 5;
			result += QueueToCombat();
			//Original state9 continues through this diagnostic after its valid queues.
		default:
			JE::LogDebugString("GameEntity::QueueToCombat() - Unknown Root Anim state currently set.\n");
		}
		value2C = 2;
		return result;
	}

	LONG GameEntity::QueueWalk() //465FC0.
	{
		LONG result = 0;
		switch (currentAnimationGroup)
		{
		case 61: case 92: case 124: case 155: case 191: case 222: case 299: return 0;
		}
		switch (value2C)
		{
		case 1:
			JE::LogDebugString("GameEntity::Queue_Walk() - Attempt to queue Walk while in a CLIMB state.\n");
			break;
		case 2:
			switch (weaponStance)
			{
			case 1:
				result = QueueAnimationPercent(59,0,1,0,0,0);
				result += QueueAnimationPercent(61,0,0,0,0,0);
				break;
			case 2:
				result = QueueAnimationPercent(90,0,1,0,0,0);
				result += QueueAnimationPercent(92,0,0,0,0,0);
				break;
			case 3:
				result = QueueAnimationPercent(122,0,1,0,0,0);
				result += QueueAnimationPercent(124,0,0,0,0,0);
				break;
			case 4:
				result = QueueAnimationPercent(153,0,1,0,0,0);
				result += QueueAnimationPercent(155,0,0,0,0,0);
				break;
			case 5:
				result = QueueAnimationPercent(189,0,1,0,0,0);
				result += QueueAnimationPercent(191,0,0,0,0,0);
				break;
			case 6:
				result = QueueAnimationPercent(220,0,1,0,0,0);
				result += QueueAnimationPercent(222,0,0,0,0,0);
				break;
			case 7:
				result = QueueAnimationPercent(295,0,1,0,0,0);
				result += QueueAnimationPercent(299,0,0,0,0,0);
				break;
			default: JE::LogDebugString("GameEntity::Queue_Walk() - Unknown weapon stance encountered.\n");
			}
			break;
		case 3: case 7: case 8:
			result = QueueToCombat();
			value2C = 2;
			result += QueueWalk();
			break;
		case 4: case 6: case 9:
			result = QueueToDefault();
			value2C = 5;
			result += QueueWalk();
			break;
		case 5:
			switch (weaponStance)
			{
			case 1:
				result = QueueAnimationPercent(369,0,1,0,0,0);
				result += QueueAnimationPercent(61,0,0,0,0,0);
				break;
			case 2:
				result = QueueAnimationPercent(375,0,1,0,0,0);
				result += QueueAnimationPercent(92,0,0,0,0,0);
				break;
			case 3:
				result = QueueAnimationPercent(381,0,1,0,0,0);
				result += QueueAnimationPercent(124,0,0,0,0,0);
				break;
			case 4:
				result = QueueAnimationPercent(387,0,1,0,0,0);
				result += QueueAnimationPercent(155,0,0,0,0,0);
				break;
			case 5:
				result = QueueAnimationPercent(393,0,1,0,0,0);
				result += QueueAnimationPercent(191,0,0,0,0,0);
				break;
			case 6:
				result = QueueAnimationPercent(399,0,1,0,0,0);
				result += QueueAnimationPercent(222,0,0,0,0,0);
				break;
			case 7:
				result = QueueAnimationPercent(430,0,1,0,0,0);
				result += QueueAnimationPercent(299,0,0,0,0,0);
				break;
			default: JE::LogDebugString("GameEntity::Queue_Walk() - Unhandled weaponstance encountered.\n");
			}
			break;
		default:
			JE::LogDebugString("GameEntity::Queue_Walk() - Unknown rootanimstate encountered.\n");
		}
		return result;
	}

	LONG GameEntity::QueueLimp() //4669F0.
	{
		LONG result = 0;
		switch (currentAnimationGroup)
		{
		case 29: case 77: case 108: case 139: case 175: case 207: case 273: return 0;
		}
		switch (value2C)
		{
		case 1:
			JE::LogDebugString("GameEntity::Queue_Limp() - Attempt to queue Limp while in a CLIMB state.\n");
			break;
		case 2:
			switch (weaponStance)
			{
			case 1:
				result = QueueAnimationPercent(57,0,1,0,0,0);
				result += QueueAnimationPercent(29,0,0,0,0,0);
				break;
			case 2:
				result = QueueAnimationPercent(88,0,1,0,0,0);
				result += QueueAnimationPercent(77,0,0,0,0,0);
				break;
			case 3:
				result = QueueAnimationPercent(120,0,1,0,0,0);
				result += QueueAnimationPercent(108,0,0,0,0,0);
				break;
			case 4:
				result = QueueAnimationPercent(151,0,1,0,0,0);
				result += QueueAnimationPercent(139,0,0,0,0,0);
				break;
			case 5:
				result = QueueAnimationPercent(187,0,1,0,0,0);
				result += QueueAnimationPercent(175,0,0,0,0,0);
				break;
			case 6:
				result = QueueAnimationPercent(218,0,1,0,0,0);
				result += QueueAnimationPercent(207,0,0,0,0,0);
				break;
			case 7:
				result = QueueAnimationPercent(293,0,1,0,0,0);
				result += QueueAnimationPercent(273,0,0,0,0,0);
				break;
			default: JE::LogDebugString("GameEntity::Queue_Limp() - Unknown weapon stance encountered.\n");
			}
			break;
		case 3: case 7: case 8:
			result = QueueToCombat();
			value2C = 2;
			result += QueueLimp();
			break;
		case 4: case 6: case 9:
			result = QueueToDefault();
			value2C = 5;
			result += QueueLimp();
			break;
		case 5:
			switch (weaponStance)
			{
			case 1:
				result = QueueAnimationPercent(367,0,1,0,0,0);
				result += QueueAnimationPercent(29,0,0,0,0,0);
				break;
			case 2:
				result = QueueAnimationPercent(373,0,1,0,0,0);
				result += QueueAnimationPercent(77,0,0,0,0,0);
				break;
			case 3:
				result = QueueAnimationPercent(379,0,1,0,0,0);
				result += QueueAnimationPercent(108,0,0,0,0,0);
				break;
			case 4:
				result = QueueAnimationPercent(385,0,1,0,0,0);
				result += QueueAnimationPercent(139,0,0,0,0,0);
				break;
			case 5:
				result = QueueAnimationPercent(391,0,1,0,0,0);
				result += QueueAnimationPercent(175,0,0,0,0,0);
				break;
			case 6:
				result = QueueAnimationPercent(397,0,1,0,0,0);
				result += QueueAnimationPercent(207,0,0,0,0,0);
				break;
			case 7:
				result = QueueAnimationPercent(426,0,1,0,0,0);
				result += QueueAnimationPercent(273,0,0,0,0,0);
				break;
			default: JE::LogDebugString("GameEntity::Queue_Limp() - Unhandled weaponstance encountered.\n");
			}
			break;
		default:
			JE::LogDebugString("GameEntity::Queue_Limp() - Unknown rootanimstate encountered.\n");
		}
		return result;
	}
}
