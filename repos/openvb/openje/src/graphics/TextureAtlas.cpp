#include "TextureAtlas.hpp"
#include <string.h>

namespace
{
	const double regions[17][4] = // Original6FB128; indices13..16 contain zeroes.
	{
		{0, 0, 0.5, 0.5},
		{0, 0.5, 0.5, 1},
		{0.5, 0, 0.75, 0.25},
		{0.75, 0, 1, 0.25},
		{0.5, 0.5, 0.75, 0.75},
		{0.75, 0.25, 1, 0.5},
		{0.75, 0.5, 1, 0.75},
		{0.5, 0.25, 0.625, 0.375},
		{0.5, 0.375, 0.5625, 0.4375},
		{0.5, 0.4375, 0.5625, 0.5},
		{0.5625, 0.4375, 0.625, 0.5},
		{0.625, 0.375, 0.75, 0.5},
		{0.625, 0.25, 0.75, 0.375},
		{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}
	};
}

BYTE Gfx_GetTextureRegion(LONG slot, double* minimum, double* maximum)
{
	if (slot < 0 || slot >= 17) return 0;
	memcpy(minimum, &regions[slot][0], 2 * sizeof(double));
	memcpy(maximum, &regions[slot][2], 2 * sizeof(double));
	return 1;
}
