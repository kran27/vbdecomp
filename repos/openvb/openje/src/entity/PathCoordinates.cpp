#include "PathManager.hpp"
#include "Map.hpp"
#include "graphics/Octree.hpp"
#include <stddef.h>

namespace JE
{
	LONG PathIntegerSquareRoot(LONG value)
	{
		LONG root;
		if (value >= 0x40000000)
		{
			value -= 0x40000000;
			root = 0x40000000;
		}
		else root = 0;
#define JE_SQRT_STEP(bit) \
		if (root + bit <= value) { value -= root; value -= bit; root |= bit * 2; } \
		root >>= 1
		JE_SQRT_STEP(0x10000000);
		JE_SQRT_STEP(0x4000000);
		JE_SQRT_STEP(0x1000000);
		JE_SQRT_STEP(0x400000);
		JE_SQRT_STEP(0x100000);
		JE_SQRT_STEP(0x40000);
		JE_SQRT_STEP(0x10000);
		JE_SQRT_STEP(0x4000);
		JE_SQRT_STEP(0x1000);
		JE_SQRT_STEP(0x400);
		JE_SQRT_STEP(0x100);
		JE_SQRT_STEP(0x40);
		JE_SQRT_STEP(0x10);
		JE_SQRT_STEP(0x4);
		JE_SQRT_STEP(0x1);
#undef JE_SQRT_STEP
		if (root < value) ++root;
		return root;
	}

	PathPoint* WorldToPathCell(PathPoint* output, const D3DXVECTOR3& position, Map* map)
	{
		float bounds[6];
		map->GetOctree()->GetLevelBounds(bounds);
		MapGrid* grid = map->terrainGrid; //5606A3 captures the live grid after bounds lookup.
		PathPoint cell;
		cell.x = static_cast<LONG>((position.x - bounds[0])
			/ (bounds[3] - bounds[0]) * static_cast<DWORD>(grid->width));
		cell.y = static_cast<LONG>((-1.0f / (bounds[5] - bounds[2]))
			* (position.z - bounds[5]) * static_cast<DWORD>(grid->height));
		//560704/706 publish only after both real CRT conversions.
		*output = cell;
		return output;
	}

	D3DXVECTOR3* PathCellToWorld(D3DXVECTOR3* output, const PathPoint& point, Map* map)
	{
		float bounds[6];
		map->GetOctree()->GetLevelBounds(bounds);
		MapGrid* grid = map->terrainGrid;
		double firstRatio = static_cast<double>(point.x) / static_cast<DWORD>(grid->width);
		DWORD height = static_cast<DWORD>(grid->height);
		double x = firstRatio * (static_cast<double>(bounds[3]) - bounds[0]) + bounds[0];
		double rowFraction = static_cast<double>(point.y) / height;
		output->y = 0.0f; //560773 precedes the final Z arithmetic and X/Z stores.
		double z = static_cast<double>(bounds[5]) - rowFraction * (static_cast<double>(bounds[5]) - bounds[2]);
		output->x = static_cast<float>(x);
		output->z = static_cast<float>(z);
		return output;
	}

	typedef char PathPointExtent[(sizeof(PathPoint) == 8) ? 1 : -1];
	typedef char PathPointFirst[(offsetof(PathPoint, x) == 0) ? 1 : -1];
	typedef char PathPointRow[(offsetof(PathPoint, y) == 4) ? 1 : -1];
}
