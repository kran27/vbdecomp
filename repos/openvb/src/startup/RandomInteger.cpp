#include "Random.hpp"

namespace F3
{
	int __cdecl RandomInteger(int low, int high)
	{
		if (low == high) return low;
		if (high < low)
		{
			low ^= high;
			high ^= low;
			low ^= high;
		}
		return int(RandomNext() % (unsigned(high) - unsigned(low) + 1u) + unsigned(low));
	}
}
