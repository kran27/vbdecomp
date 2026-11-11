#include "PathManager.hpp"
#include "Entity.hpp"
#include "Map.hpp"
#include "World.hpp"
#include "graphics/Octree.hpp"
#include <set>
#include <stdexcept>

namespace JE
{
	BYTE PathManager::Neighbor(DWORD direction, LONG x, LONG y, PathPoint& output, PathRequest* request)
	{
		output.x = x; output.y = y;
		switch (direction)
		{
		case 0: output.x -= 3; output.y -= 3; break;
		case 1: output.y -= 3; break;
		case 2: output.x += 3; output.y -= 3; break;
		case 3: output.x -= 3; break;
		case 4: output.x += 3; break;
		case 5: output.x -= 3; output.y += 3; break;
		case 6: output.y += 3; break;
		case 7: output.x += 3; output.y += 3; break;
		}
		MapGrid* terrain = request->terrain;
		if (output.x < 0 || output.y < 0 || output.x >= terrain->width || output.y >= terrain->height) return 0;
		const std::vector<RLESpan*>& spans = request->owner->spatialFootprint->spans;
		for (DWORD index = 0; index < spans.size(); ++index)
		{
			RLESpan* span = spans[index];
			if (!span) continue;
			LONG first = output.x + span->first - request->startCell.x;
			LONG row = output.y + span->row - request->startCell.y;
			LONG count = span->count;
			if (row < 0 || row >= terrain->height || first >= terrain->width || first+count <= 0) continue;
			if (first < 0) { count += first; first = 0; }
			else if (first+count >= terrain->width) count = terrain->width-first;
			LONG end = first+count;
			while (first < end)
			{
				RLECell<BYTE>* cell = terrain->GetCell(first, row);
				if (IsBlocked(cell->values.front(), request->owner)) return 0;
				first = cell->first + cell->count;
			}
		}
		return 1;
	}

	BYTE PathManager::CanTraverse(PathRequest* request, D3DXVECTOR3 from, D3DXVECTOR3 to)
	{
		std::set<BYTE> flags;
		D3DXVECTOR3 difference;
		difference.z = to.z - from.z;
		difference.y = to.y - from.y;
		const double differenceX = double(to.x) - from.x;
		difference.x = static_cast<float>(differenceX);
		//562698 stores interpolation X without popping the wider distance value.
		LONG squared = static_cast<LONG>(static_cast<__int64>(differenceX * differenceX
			+ double(difference.z) * difference.z));
		float samples = PathIntegerSquareRoot(squared) * 1.666f;
		if (samples <= 1.0f) return 1;
		const float inverse = static_cast<float>(1.0 / samples);
		for (LONG sample = 1; static_cast<double>(sample) < samples; ++sample)
		{
			double fraction = static_cast<double>(sample) * inverse;
			//562721/72B/735 round all products before adding the origin.
			volatile float z = static_cast<float>(difference.z * fraction);
			volatile float y = static_cast<float>(difference.y * fraction);
			volatile float x = static_cast<float>(difference.x * fraction);
			D3DXVECTOR3 position;
			position.x = x + from.x;
			position.y = from.y + y;
			position.z = z + from.z;
			IWorld* world = GetWorldInstance();
			Map* map = world->GetMap(request->owner->mapHandle);
			PathPoint point;
			WorldToPathCell(&point, position, map);
			request->terrain->CollectFootprintValues(flags, request->owner->spatialFootprint->spans,
				request->startCell.x, request->startCell.y, point.x, point.y);
			for (std::set<BYTE>::const_iterator value = flags.begin(); value != flags.end(); ++value)
				if (IsBlocked(*value, request->owner)) return 0;
		}
		return 1;
	}

	BYTE PathManager::InitializeRequest(PathRequest* request)
	{
		IWorld* world = GetWorldInstance();
		Map* map = world->GetMap(request->owner->mapHandle);
		float bounds[6]; map->GetOctree()->GetLevelBounds(bounds);
		if (!(request->goal.x >= bounds[0] && request->goal.x <= bounds[3]
			&& request->goal.z >= bounds[2] && request->goal.z <= bounds[5])) return 0;
		PathPoint cell;
		request->startCell = *WorldToPathCell(&cell, request->start, map);
		request->goalCell = *WorldToPathCell(&cell, request->goal, map);
		if (request->owner->MatchesPathCell(request->goalCell)) return 0;
		DWORD direction;
		if (request->startCell.x < request->goalCell.x)
			direction = request->startCell.y < request->goalCell.y ? 0 : (request->startCell.y == request->goalCell.y ? 3 : 5);
		else if (request->startCell.x > request->goalCell.x)
			direction = request->startCell.y < request->goalCell.y ? 2 : (request->startCell.y == request->goalCell.y ? 4 : 7);
		else
		{
			if (request->startCell.y == request->goalCell.y) return 1;
			direction = request->startCell.y < request->goalCell.y ? 1 : 6;
		}
		std::set<BYTE> flags;
		request->terrain->CollectFootprintValues(flags, request->owner->spatialFootprint->spans,
			request->startCell.x, request->startCell.y, request->goalCell.x, request->goalCell.y);
		BYTE blocked = 0;
		for (std::set<BYTE>::const_iterator value = flags.begin(); value != flags.end(); ++value)
			if (IsBlocked(*value, request->owner)) blocked = 1;
		if (!blocked) return 1;
		if (request->adjustGoal)
		{
			PathPoint point = request->goalCell;
			for (DWORD attempt = 0; attempt < 10; ++attempt)
			{
				PathPoint next;
				BYTE allowed = Neighbor(direction, point.x, point.y, next, request);
				point = next;
				if (allowed)
				{
					IWorld* adjustedWorld = GetWorldInstance();
					Map* goalMap = adjustedWorld->GetMap(request->owner->mapHandle);
					request->goalCell = point;
					D3DXVECTOR3 goal;
					request->goal = *PathCellToWorld(&goal, point, goalMap);
					return 1;
				}
			}
		}
		return 0;
	}
}
