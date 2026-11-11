#pragma once

#include <windows.h>
#include <d3dx8math.h>
#include <string>

namespace JE
{
	class NavPoint
	{
	public:
		NavPoint(); //5B2BA0.
		virtual ~NavPoint(); //5B2BD0, no owned links.
		DWORD enabled;
		D3DXVECTOR3 position, orientation;
		NavPoint* links[5]; //Borrowed pointers, file bytes admit only0..4.
	};
	class WayPoint
	{
	public:
		WayPoint(); //5B2C00.
		virtual ~WayPoint(); //5B2C30, normal string destruction.
		std::string name;
		D3DXVECTOR3 position, orientation;
		WayPoint* previous;
		WayPoint* next; //Borrowed circular links within the owning path.
	};
	struct WayPointPath
	{
		std::string name;
		LONG count;
		WayPoint** points; //Map::ClearWaypoints owns points and this array.
	};
}
