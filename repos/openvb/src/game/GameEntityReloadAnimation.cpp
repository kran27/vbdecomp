#include "GameEntity.hpp"

namespace F3
{
	LONG GameEntity::QueueReload() //4659A0, actual animation event120.
	{
		LONG transition = 0;
		switch (value2C)
		{
		case 2: case 5:
			switch (weaponStance)
			{
			case 1: transition = QueueAnimationPercent(32,0,1,0,0,0); break;
			case 2: transition = QueueAnimationPercent(80,0,1,0,0,0); break;
			case 3: transition = QueueAnimationPercent(111,0,1,0,0,0); break;
			case 4: transition = QueueAnimationPercent(142,0,1,0,0,0); break;
			case 5: transition = QueueAnimationPercent(178,0,1,0,0,0); break;
			case 6: transition = QueueAnimationPercent(210,0,1,0,0,0); break;
			}
			break;
		case 3: case 6: case 10: break;
		default:
			transition = QueueToCombat();
			value2C = 2;
			transition = LONG(DWORD(transition) + DWORD(QueueReload()));
			break;
		}
		return transition;
	}
}
