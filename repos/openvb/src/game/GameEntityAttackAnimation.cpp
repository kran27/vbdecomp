#include "GameEntity.hpp"

namespace F3
{
	LONG GameEntity::QueueAttackAnimation(DWORD event)
	{
		LONG result = 0;
		switch (value2C)
		{
		case 3:
			switch (event)
			{
			case 53:
				if (weaponStance == 7) result = QueueAnimationPercent(333,0,1,0,0,0);
				else result = QueueAnimationPercent(305,0,1,0,0,0);
				break;
			case 54:
				if (weaponStance == 7) result = QueueAnimationPercent(334,0,1,0,0,0);
				else result = QueueAnimationPercent(306,0,1,0,0,0);
				break;
			case 56: result = QueueAnimationPercent(307,0,1,0,0,0); break;
			case 57:
				if (weaponStance == 7) result = QueueAnimationPercent(340,0,1,0,0,0);
				else result = QueueAnimationPercent(313,0,1,0,0,0);
				break;
			case 58: result = QueueAnimationPercent(322,0,1,0,0,0); break;
			case 59: result = QueueAnimationPercent(323,0,1,0,0,0); break;
			case 61: case 62: result = QueueAnimationPercent(325,0,1,0,0,0); break;
			case 63: result = QueueAnimationPercent(326,0,1,0,0,0); break;
			}
			break;
		case 2:
			switch (event)
			{
			case 53:
				if (weaponStance == 7) result = QueueAnimationPercent(229,0,1,0,0,0);
				else result = QueueAnimationPercent(7,0,1,0,0,0);
				break;
			case 54:
				if (weaponStance == 7) result = QueueAnimationPercent(230,0,1,0,0,0);
				else result = QueueAnimationPercent(8,0,1,0,0,0);
				break;
			case 55:
				if (weaponStance == 7) result = QueueAnimationPercent(231,0,1,0,0,0);
				else result = QueueAnimationPercent(9,0,1,0,0,0);
				break;
			case 56:
				switch (weaponStance) { case 1: result = QueueAnimationPercent(11,0,1,0,0,0); break; case 3: result = QueueAnimationPercent(97,0,1,0,0,0); break; case 5: result = QueueAnimationPercent(160,0,1,0,0,0); break; }
				break;
			case 57:
				switch (weaponStance)
				{
				case 1: result = QueueAnimationPercent(22,0,1,0,0,0); break;
				case 2: result = QueueAnimationPercent(72,0,1,0,0,0); break;
				case 3: result = QueueAnimationPercent(103,0,1,0,0,0); break;
				case 4: result = QueueAnimationPercent(134,0,1,0,0,0); break;
				case 5: result = QueueAnimationPercent(166,0,1,0,0,0); break;
				case 6: result = QueueAnimationPercent(202,0,1,0,0,0); break;
				case 7: result = QueueAnimationPercent(247,0,1,0,0,0); break;
				}
				break;
			case 58:
				switch (weaponStance)
				{
				case 1: result = QueueAnimationPercent(33,0,1,0,0,0); break;
				case 3: result = QueueAnimationPercent(112,0,1,0,0,0); break;
				case 5: result = QueueAnimationPercent(179,0,1,0,0,0); break;
				case 6: result = QueueAnimationPercent(211,0,1,0,0,0); break;
				}
				break;
			case 59:
				switch (weaponStance)
				{
				case 1: result = QueueAnimationPercent(43,0,1,0,0,0); break;
				case 3: result = QueueAnimationPercent(116,0,1,0,0,0); break;
				case 5: result = QueueAnimationPercent(183,0,1,0,0,0); break;
				case 6: result = QueueAnimationPercent(215,0,1,0,0,0); break;
				}
				break;
			case 60:
				switch (weaponStance) { case 1: result = QueueAnimationPercent(50,0,1,0,0,0); break; case 2: result = QueueAnimationPercent(84,0,1,0,0,0); break; case 4: result = QueueAnimationPercent(146,0,1,0,0,0); break; }
				break;
			case 61: case 62:
				switch (weaponStance) { case 1: result = QueueAnimationPercent(52,0,1,0,0,0); break; case 3: result = QueueAnimationPercent(118,0,1,0,0,0); break; case 5: result = QueueAnimationPercent(185,0,1,0,0,0); break; }
				break;
			case 63:
				switch (weaponStance) { case 1: result = QueueAnimationPercent(53,0,1,0,0,0); break; case 4: result = QueueAnimationPercent(148,0,1,0,0,0); break; }
				break;
			case 64:
				switch (weaponStance) { case 1: result = QueueAnimationPercent(54,0,1,0,0,0); break; case 2: result = QueueAnimationPercent(86,0,1,0,0,0); break; case 4: result = QueueAnimationPercent(149,0,1,0,0,0); break; }
				break;
			case 65:
				switch (weaponStance) { case 1: result = QueueAnimationPercent(64,0,1,0,0,0); break; case 5: result = QueueAnimationPercent(194,0,1,0,0,0); break; }
				break;
			case 66: result = QueueAnimationPercent(13,0,1,0,0,0); break;
			case 67: result = QueueAnimationPercent(15,0,1,0,0,0); break;
			case 68: result = QueueAnimationPercent(14,0,1,0,0,0); break;
			case 69: result = QueueAnimationPercent(16,0,1,0,0,0); break;
			case 70: result = QueueAnimationPercent(34,0,1,0,0,0); break;
			case 71: result = QueueAnimationPercent(35,0,1,0,0,0); break;
			case 72: result = QueueAnimationPercent(36,0,1,0,0,0); break;
			case 73: result = QueueAnimationPercent(38,0,1,0,0,0); break;
			case 74: result = QueueAnimationPercent(37,0,1,0,0,0); break;
			case 75: result = QueueAnimationPercent(39,0,1,0,0,0); break;
			case 76: result = QueueAnimationPercent(44,0,1,0,0,0); break;
			case 77: result = QueueAnimationPercent(45,0,1,0,0,0); break;
			case 78: result = QueueAnimationPercent(46,0,1,0,0,0); break;
			case 79: result = QueueAnimationPercent(48,0,1,0,0,0); break;
			case 80: result = QueueAnimationPercent(47,0,1,0,0,0); break;
			case 81: result = QueueAnimationPercent(49,0,1,0,0,0); break;
			case 82: result = QueueAnimationPercent(242,0,1,0,0,0); break;
			case 83: result = QueueAnimationPercent(248,0,1,0,0,0); break;
			case 84: result = QueueAnimationPercent(249,0,1,0,0,0); break;
			case 85: result = QueueAnimationPercent(259,0,1,0,0,0); break;
			case 86: result = QueueAnimationPercent(260,0,1,0,0,0); break;
			case 87: result = QueueAnimationPercent(261,0,1,0,0,0); break;
			case 88: result = QueueAnimationPercent(262,0,1,0,0,0); break;
			case 89: result = QueueAnimationPercent(263,0,1,0,0,0); break;
			case 90: result = QueueAnimationPercent(264,0,1,0,0,0); break;
			case 91: result = QueueAnimationPercent(265,0,1,0,0,0); break;
			case 92: result = QueueAnimationPercent(266,0,1,0,0,0); break;
			case 93: result = QueueAnimationPercent(267,0,1,0,0,0); break;
			case 94: result = QueueAnimationPercent(268,0,1,0,0,0); break;
			case 95: result = QueueAnimationPercent(276,0,1,0,0,0); break;
			case 96: result = QueueAnimationPercent(277,0,1,0,0,0); break;
			case 97: result = QueueAnimationPercent(278,0,1,0,0,0); break;
			case 98: result = QueueAnimationPercent(279,0,1,0,0,0); break;
			case 99: result = QueueAnimationPercent(280,0,1,0,0,0); break;
			case 100: result = QueueAnimationPercent(281,0,1,0,0,0); break;
			case 101: result = QueueAnimationPercent(282,0,1,0,0,0); break;
			case 102: result = QueueAnimationPercent(283,0,1,0,0,0); break;
			case 103: result = QueueAnimationPercent(284,0,1,0,0,0); break;
			case 104: result = QueueAnimationPercent(285,0,1,0,0,0); break;
			}
			break;
		}
		return result;
	}
}
