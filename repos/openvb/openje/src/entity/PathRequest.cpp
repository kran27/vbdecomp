#include "PathManager.hpp"
#include "Entity.hpp"
#include "Map.hpp"
#include "World.hpp"
#include <climits>
#include <stdexcept>

namespace JE
{
	PathNode::PathNode()
	{
		point.x = 0;
		point.y = 0;
	}

	size_t PathPointHash::operator()(const PathPoint& point) const
	{
		return static_cast<DWORD>(point.x >> 1) + static_cast<DWORD>(point.y);
	}
	bool PathPointHash::operator()(const PathPoint& left, const PathPoint& right) const
	{
		return (*this)(left) < (*this)(right);
	}

	PathRequest::PathRequest(Entity* entity, D3DXVECTOR3 destination, BYTE adjust,
		DWORD value, BYTE flag) : status(0), route(NULL), owner(entity), initialized(0),
		flag39(flag), nodes(NULL), usedNodes(0), frontier(NULL), lookup(NULL), value58(value), adjustGoal(adjust)
	{
		startCell.x = startCell.y = goalCell.x = goalCell.y = 0;
		start = entity->position;
		goal = destination;
		Map* map = g_World->GetMap(entity->mapHandle);
		terrain = map->terrainGrid;
		occupancy = map->entityGrid;
		alternateTerrain = map->alternateTerrain;
	}

	PathRequest::~PathRequest()
	{
		if (!status)
		{
			IWorldServer* world=GetWorldServerInstance();
			world->GetPathManager()->Cancel(this);
		}
		if (route)
		{
			route->clear();
			delete route;
		}
		if (nodes) delete[] nodes;
		if (frontier)
		{
			frontier->clear();
			delete frontier;
		}
		if (lookup)
		{
			lookup->clear();
			delete lookup;
		}
	}

	PathNode* PathRequest::AcquireNode() //560460.
	{
		if (usedNodes >= 10000) return NULL;
		return nodes + usedNodes++;
	}

	PathNode* PathRequest::GetNode(LONG x, LONG y)
	{
		PathPoint point; point.x = x; point.y = y;
		std::pair<PathNodeLookup::iterator, PathNodeLookup::iterator> range = lookup->equal_range(point);
		for (PathNodeLookup::iterator entry = range.first; entry != range.second; ++entry)
		{
			if (entry->first.x == x && entry->first.y == y) return entry->second;
		}
		PathNode* node = AcquireNode();
		if (!node) return NULL;
		node->point = point;
		node->parent = NULL;
		node->open = node->closed = 0;
		node->cost = node->totalCost = LONG_MAX;
		lookup->insert(std::make_pair(point, node));
		return node;
	}
}
