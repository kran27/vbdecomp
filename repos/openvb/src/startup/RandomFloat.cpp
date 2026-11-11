#include "Random.hpp"
#include <math.h>

namespace F3
{
	float __cdecl RandomFloat(float low, float high)
	{
		if (low == high) return low;
		if (high < low)
		{
			float swap = low;
			low = high;
			high = swap;
		}
		return float(fmod(float(RandomNext()), high - low + 1.0f) + low);
	}
}
