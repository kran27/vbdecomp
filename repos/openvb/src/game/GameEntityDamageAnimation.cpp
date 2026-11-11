#include "GameEntity.hpp"

namespace F3
{
	LONG GameEntity::QueueDamageReaction(DWORD event) //464D70.
	{
		LONG result = 0;
		switch (value2C)
		{
		case 1:
			switch (event)
			{
			case 33: case 34: case 37: case 40: result = QueueAnimationPercent(1,0,1,0,0,0); break;
			case 38: case 41: result = QueueAnimationPercent(2,0,1,0,0,0); break;
			}
			break;
		case 2: case 5:
			switch (weaponStance)
			{
			case 1:
				switch (event)
				{
				case 37: result = QueueAnimationPercent(19,0,1,0,0,0); break;
				case 40: result = QueueAnimationPercent(17,0,1,0,0,0); break;
				case 33: result = QueueAnimationPercent(63,0,1,0,0,0); break;
				case 34: result = QueueAnimationPercent(62,0,1,0,0,0); break;
				case 38: result = QueueAnimationPercent(20,0,1,0,0,0); break;
				case 41: result = QueueAnimationPercent(18,0,1,0,0,0); break;
				}
				break;
			case 2:
				switch (event)
				{
				case 37: result = QueueAnimationPercent(70,0,1,0,0,0); break;
				case 40: result = QueueAnimationPercent(68,0,1,0,0,0); break;
				case 33: result = QueueAnimationPercent(94,0,1,0,0,0); break;
				case 34: result = QueueAnimationPercent(93,0,1,0,0,0); break;
				case 38: result = QueueAnimationPercent(71,0,1,0,0,0); break;
				case 41: result = QueueAnimationPercent(69,0,1,0,0,0); break;
				}
				break;
			case 3:
				switch (event)
				{
				case 37: result = QueueAnimationPercent(101,0,1,0,0,0); break;
				case 40: result = QueueAnimationPercent(99,0,1,0,0,0); break;
				case 33: result = QueueAnimationPercent(126,0,1,0,0,0); break;
				case 34: result = QueueAnimationPercent(125,0,1,0,0,0); break;
				case 38: result = QueueAnimationPercent(102,0,1,0,0,0); break;
				case 41: result = QueueAnimationPercent(100,0,1,0,0,0); break;
				}
				break;
			case 4:
				switch (event)
				{
				case 37: result = QueueAnimationPercent(132,0,1,0,0,0); break;
				case 40: result = QueueAnimationPercent(130,0,1,0,0,0); break;
				case 33: result = QueueAnimationPercent(157,0,1,0,0,0); break;
				case 34: result = QueueAnimationPercent(156,0,1,0,0,0); break;
				case 38: result = QueueAnimationPercent(133,0,1,0,0,0); break;
				case 41: result = QueueAnimationPercent(131,0,1,0,0,0); break;
				}
				break;
			case 5:
				switch (event)
				{
				case 37: result = QueueAnimationPercent(164,0,1,0,0,0); break;
				case 40: result = QueueAnimationPercent(162,0,1,0,0,0); break;
				case 33: result = QueueAnimationPercent(193,0,1,0,0,0); break;
				case 34: result = QueueAnimationPercent(192,0,1,0,0,0); break;
				case 38: result = QueueAnimationPercent(165,0,1,0,0,0); break;
				case 41: result = QueueAnimationPercent(163,0,1,0,0,0); break;
				}
				break;
			case 6:
				switch (event)
				{
				case 37: result = QueueAnimationPercent(200,0,1,0,0,0); break;
				case 40: result = QueueAnimationPercent(198,0,1,0,0,0); break;
				case 33: result = QueueAnimationPercent(224,0,1,0,0,0); break;
				case 34: result = QueueAnimationPercent(223,0,1,0,0,0); break;
				case 38: result = QueueAnimationPercent(201,0,1,0,0,0); break;
				case 41: result = QueueAnimationPercent(199,0,1,0,0,0); break;
				}
				break;
			default:
				switch (event)
				{
				case 37: result = QueueAnimationPercent(245,0,1,0,0,0); break;
				case 38: result = QueueAnimationPercent(246,0,1,0,0,0); break;
				case 40: result = QueueAnimationPercent(243,0,1,0,0,0); break;
				case 41: result = QueueAnimationPercent(244,0,1,0,0,0); break;
				case 33: result = QueueAnimationPercent(301,0,1,0,0,0); break;
				case 34: result = QueueAnimationPercent(300,0,1,0,0,0); break;
				}
				break;
			}
			break;
		case 3: case 6:
			switch (weaponStance)
			{
			default:
				switch (event)
				{
				case 37: case 33: result = QueueAnimationPercent(338,0,1,0,0,0); break;
				case 38: result = QueueAnimationPercent(339,0,1,0,0,0); break;
				case 40: case 34: result = QueueAnimationPercent(336,0,1,0,0,0); break;
				case 41: result = QueueAnimationPercent(337,0,1,0,0,0); break;
				}
				break;
			case 1:
				switch (event)
				{
				case 37: result = QueueAnimationPercent(311,0,1,0,0,0); break;
				case 38: result = QueueAnimationPercent(312,0,1,0,0,0); break;
				case 40: result = QueueAnimationPercent(309,0,1,0,0,0); break;
				case 41: result = QueueAnimationPercent(310,0,1,0,0,0); break;
				case 33: result = QueueAnimationPercent(317,0,1,0,0,0); break;
				case 34: result = QueueAnimationPercent(316,0,1,0,0,0); break;
				}
				break;
			}
			// Original roots3/6 continue with the crouched reaction.
		case 4:
			switch (event)
			{
			case 33: case 34: case 37: case 40: result += QueueAnimationPercent(353,0,1,0,0,0); break;
			case 38: case 41: result += QueueAnimationPercent(354,0,1,0,0,0); break;
			}
			break;
		case 7:
			switch (event)
			{
			case 33: case 34: case 37: case 40: result = QueueAnimationPercent(485,0,1,0,0,0); break;
			case 38: case 41: result = QueueAnimationPercent(486,0,1,0,0,0); break;
			}
			break;
		case 8:
			switch (event)
			{
			case 33: case 34: case 37: case 40: result = QueueAnimationPercent(500,0,1,0,0,0); break;
			case 38: case 41: result = QueueAnimationPercent(501,0,1,0,0,0); break;
			}
			break;
		case 9:
			switch (event)
			{
			case 33: case 34: case 37: case 40: result = QueueAnimationPercent(505,0,1,0,0,0); break;
			case 38: case 41: result = QueueAnimationPercent(506,0,1,0,0,0); break;
			}
			break;
		case 10: break;
		default:
			value2C = 2;
			result = QueueDamageReaction(event);
		}
		return result;
	}
}
