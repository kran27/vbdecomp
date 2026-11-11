#include "GameEntity.hpp"

namespace F3
{
	LONG GameEntity::QueuePickup() //466880, actual animation event141.
	{
		LONG duration = 0;
		switch (value2C)
		{
		case 5: duration = QueueAnimationPercent(435,0,1,0,0,0); break;
		case 2: duration = QueueAnimationPercent(302,0,1,0,0,0); break;
		case 4: case 6: case 10: break;
		default:
			duration = QueueToDefault();
			value2C = 5;
			duration = LONG(DWORD(duration) + DWORD(QueuePickup()));
			break;
		}
		return duration;
	}
}
