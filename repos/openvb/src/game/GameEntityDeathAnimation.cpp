#include "GameEntity.hpp"

namespace F3
{
	LONG GameEntity::QueueDeathAnimation(DWORD event) //464940.
	{
		LONG result = 0;
		switch (value2C)
		{
		case 2:
			switch (event)
			{
			case 44:
				result = QueueAnimationPercent(233,0,1,0,0.0f,1);
				break;
			case 45:
				result = QueueAnimationPercent(234,0,1,0,0.0f,1);
				break;
			case 46:
				result = QueueAnimationPercent(235,0,1,0,0.0f,1);
				break;
			case 47:
				result = QueueAnimationPercent(236,0,1,0,0.0f,1);
				break;
			case 48:
				result = QueueAnimationPercent(237,0,1,0,0.0f,1);
				break;
			case 49:
				result = QueueAnimationPercent(238,0,1,0,0.0f,1);
				break;
			case 50:
				result = QueueAnimationPercent(239,0,1,0,0.0f,1);
				break;
			case 51:
				result = QueueAnimationPercent(240,0,1,0,0.0f,1);
				break;
			case 52:
				result = QueueAnimationPercent(241,0,1,0,0.0f,1);
				break;
			default:
				result = QueueAnimationPercent(21,0,1,0,0.0f,1);
				break;
			}
			value2C = 10;
			break;
		case 7:
			result = QueueAnimationPercent(484,0,1,0,0.0f,1);
			value2C = 10;
			break;
		case 8:
			result = QueueAnimationPercent(499,0,1,0,0.0f,1);
			value2C = 10;
			break;
		case 10: break;
		default:
			value2C = 2;
			result = QueueDeathAnimation(event);
			break;
		}
		return result;
	}

	LONG GameEntity::QueueProneBack(BYTE knock) //464B80.
	{
		LONG result = 0;
		switch (value2C)
		{
		case 2:
			result = QueueAnimationPercent(knock ? 269 : 271,0,1,0,0.0f,0);
			value2C = 7;
			break;
		case 3:
			result = QueueAnimationPercent(knock ? 344 : 346,0,1,0,0.0f,0);
			value2C = 7;
			break;
		case 4:
			result = QueueAnimationPercent(356,0,1,0,0.0f,0);
			value2C = 7;
			break;
		case 5:
			value2C = 2;
			result = QueueProneBack(knock);
			break;
		case 7: case 8: case 10: break;
		default:
			value2C = 2;
			result = QueueProneBack(knock);
			break;
		}
		return result;
	}

	LONG GameEntity::QueueProneFront(BYTE knock) //464C70.
	{
		LONG duration = 0;
		switch (value2C)
		{
		case 2:
			duration = QueueAnimationPercent(knock ? 270 : 272,0,1,0,0.0f,0);
			value2C = 8;
			break;
		case 3:
			duration = QueueAnimationPercent(knock ? 345 : 347,0,1,0,0.0f,0);
			value2C = 8;
			//Original464CF4 falls through and queues crouched-prone357 too.
		case 4:
			duration = LONG(DWORD(duration) + DWORD(QueueAnimationPercent(357,0,1,0,0.0f,0)));
			value2C = 8;
			break;
		case 7: case 8: case 10: break;
		default:
			value2C = 2;
			duration = QueueProneFront(knock);
			break;
		}
		return duration;
	}
}
