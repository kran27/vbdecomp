#include "MapNavigation.hpp"
#include "Map.hpp"
#include "resources/ResourceSystem.hpp"
#include <new>
#include <cstring>

namespace JE
{
	NavPoint::NavPoint() : position(0,0,0), orientation(0,0,0)
	{
		enabled = 0;
		for (DWORD index = 0; index < 5; ++index) links[index] = NULL;
	}
	NavPoint::~NavPoint() {}
	WayPoint::WayPoint() : position(0,0,0), orientation(0,0,0) { previous = next = NULL; }
	WayPoint::~WayPoint() {}

	DWORD Map::GetWayPointPathCount() const
	{
		return waypoints.size();
	}
	WayPointPath* Map::GetWayPointPath(LONG index)
	{
		if (index < 0 || index >= LONG(GetWayPointPathCount())) return NULL;
		std::map<std::string,WayPointPath>::iterator i = waypoints.begin();
		while (index > 0) { ++i; --index; }
		return &i->second;
	}

	DWORD Map::GetWayPointPosition(const std::string& name,DWORD index,D3DXVECTOR3& output)
	{
		WayPointPath* path=FindWayPointPath(name);
		if (!path) return 0;
		if (index<DWORD(path->count))
		{
			output=path->points[index]->position;
			return 1;
		}
		return 0;
	}

	DWORD Map::GetWayPointOrientation(const std::string& name,DWORD index,D3DXVECTOR3& output)
	{
		WayPointPath* path=FindWayPointPath(name);
		if (!path) return 0;
		if (index<DWORD(path->count))
		{
			output=path->points[index]->orientation;
			return 1;
		}
		return 0;
	}

	DWORD Map::GetWayPointCount(const std::string& name)
	{
		WayPointPath* path=FindWayPointPath(name);
		if (!path) return 0;
		return DWORD(path->count);
	}

	DWORD Map::GetNavPointCount() const
	{
		return navigation.size();
	}
	DWORD Map::GetNavPointLinks(DWORD index, DWORD* output) const
	{
		if (index > navigation.size()) return 0;
		NavPoint** links = navigation[index]->links;
		DWORD count = 0;
		DWORD left = 5;
		do
		{
			NavPoint* target = *links;
			if (!target) *output = DWORD(-1);
			else
			{
				for (DWORD ordinal = 0; ordinal < navigation.size(); ++ordinal)
				{
					if (navigation[ordinal] == target)
					{
						*output = ordinal;
						++count;
						break;
					}
				}
			}
			++links;
			++output;
		} while (--left);
		return count;
	}
	DWORD Map::GetNavPointPosition(DWORD index, D3DXVECTOR3& output) const
	{
		if (index > navigation.size()) return 0;
		NavPoint* point = navigation[index];
		output = point->position;
		return 1;
	}
	DWORD Map::GetNavPointOrientation(DWORD index, D3DXVECTOR3& output) const
	{
		if (index > navigation.size()) return 0;
		NavPoint* point = navigation[index];
		output = point->orientation;
		return 1;
	}

	BYTE Map::ReadNavigation(Common::Chunk&, Resources::ResourceMemoryStream* stream)
	{
		LONG count;
		if (!Resources::g_pResourceSystem->ReadMemory(&count, 4, stream)) return 0;
		NavPoint** points = new NavPoint*[count];
		for (LONG index = 0; index < count; ++index) points[index] = new NavPoint;
		for (LONG index = 0; index < count; ++index)
		{
			NavPoint* point = points[index];
			BYTE enabled;
			if (!Resources::g_pResourceSystem->ReadMemory(&enabled, 1, stream)) return 0;
			point->enabled = enabled == 1;
			D3DXVECTOR3 value;
			if (!Resources::g_pResourceSystem->ReadMemory(&value.x, 4, stream) || !Resources::g_pResourceSystem->ReadMemory(&value.y, 4, stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&value.z, 4, stream)) return 0;
			point->position = value;
			if (!Resources::g_pResourceSystem->ReadMemory(&value.x, 4, stream) || !Resources::g_pResourceSystem->ReadMemory(&value.y, 4, stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&value.z, 4, stream)) return 0;
			point->orientation = value;
			NavPoint** nextLink = point->links;
			for (LONG link = 0; link < 5; ++link, ++nextLink)
			{
				signed char ordinal;
				if (!Resources::g_pResourceSystem->ReadMemory(&ordinal, 1, stream)) return 0;
				//Original tests0<=ordinal<5, not ordinal<count.
				*nextLink = ordinal >= 0 && ordinal < 5 ? points[ordinal] : NULL;
			}
			navigation.push_back(point);
		}
		delete[] points;
		return 1; //Failure paths retain the original unpublished-allocation leaks.
	}

	BYTE Map::ReadWaypoints(Common::Chunk&, Resources::ResourceMemoryStream* stream)
	{
		std::string name;
		LONG count;
		if (!Common::ReadString(name, stream) || !Resources::g_pResourceSystem->ReadMemory(&count, 4, stream)) return 0;
		if (count <= 0) return 1;
		WayPoint** points = new WayPoint*[count];
		for (LONG index = 0; index < count; ++index)
		{
			WayPoint* point = new WayPoint;
			points[index] = point;
			D3DXVECTOR3 value;
			if (!Resources::g_pResourceSystem->ReadMemory(&value.x, 4, stream) || !Resources::g_pResourceSystem->ReadMemory(&value.y, 4, stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&value.z, 4, stream)) return 0;
			point->position = value;
			if (!Resources::g_pResourceSystem->ReadMemory(&value.x, 4, stream) || !Resources::g_pResourceSystem->ReadMemory(&value.y, 4, stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&value.z, 4, stream)) return 0;
			point->orientation = value;
		}
		for (LONG index = 0; index < count; ++index)
		{
			points[index]->previous = points[index ? index-1 : count-1];
			points[index]->next = points[index == count-1 ? 0 : index+1];
		}
		WayPointPath path;
		path.name = name;
		path.count = count;
		path.points = points;
		waypoints[name] = path; //Original duplicate-key replacement does not free old points.
		return 1;
	}

	void Map::ClearNavigation()
	{
		for (std::vector<NavPoint*>::iterator i = navigation.begin(); i != navigation.end(); ++i) delete *i;
		navigation.clear(); //Supplied VC7 clear also releases vector storage.
	}
	void Map::ClearWaypoints()
	{
		for (std::map<std::string,WayPointPath>::iterator i = waypoints.begin(); i != waypoints.end(); ++i)
		{
			WayPointPath& path = i->second;
			for (LONG index = 0; index < path.count; ++index) delete path.points[index];
			delete[] path.points;
		}
		waypoints.clear();
	}
}
