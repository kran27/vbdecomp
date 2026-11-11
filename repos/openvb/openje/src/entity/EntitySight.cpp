#include "Entity.hpp"
#include "Map.hpp"
#include "MapFog.hpp"
#include "World.hpp"
#include "model/ModelInstance.hpp"

namespace JE
{
	float Entity::GetModelHeight() const //548FF0.
	{
		float height = 0.0f;
		if (model)
		{
			D3DXVECTOR3 minimum, maximum;
			model->GetBounds(minimum, maximum);
			height = maximum.y - minimum.y;
		}
		return height;
	}
	DWORD Entity::GetLOSCell(LONG first, LONG row, const float* targetHeight)
	{
		if (!mapHandle) return 0;
		IWorld* world = g_World;
		Map* map = world->GetMap(mapHandle);
		if (!map) return 0;
		AreaMap* fog = map->areaMap;
		if (!fog) return 0;
		D3DXVECTOR3 origin;
		if (model && model->IsAttached())
		{
			D3DXVECTOR3 attached;
			model->GetAttachedPosition(attached);
			origin = D3DXVECTOR3(attached.x, attached.y, attached.z);
		}
		else origin = D3DXVECTOR3(position.x, position.y, position.z);
		LONG fromFirst, fromRow;
		fog->WorldToCell(origin.x, origin.y, origin.z, &fromFirst, &fromRow);
		return fog->TestLOS(fromFirst, fromRow, float(GetModelHeight() + origin.y), first, row, targetHeight) == 1 ? 2 : 0;
	}
	DWORD Entity::GetLOSPoint(float x, float y, float z)
	{
		Map* map = NULL;
		if (mapHandle)
		{
			IWorld* world = g_World;
			map = world->GetMap(mapHandle);
		}
		if (!map || !map->areaMap) return 0;
		LONG first, row;
		map->areaMap->WorldToCell(x, y, z, &first, &row);
		return GetLOSCell(first, row, &y);
	}
	DWORD Entity::GetLOS(Entity* target)
	{
		DWORD sourceMap=mapHandle;
		if (!sourceMap) return 0;
		IWorld* world=g_World;
		Map* map=world->GetMap(sourceMap);
		if (!map || target->mapHandle != sourceMap) return 0;
		AreaMap* fog=map->areaMap;
		if (!fog) return 0;
		D3DXVECTOR3 targetPosition;
		if (target->model && target->model->IsAttached())
		{
			D3DXVECTOR3 attached;
			target->model->GetAttachedPosition(attached);
			targetPosition=D3DXVECTOR3(attached.x,attached.y,attached.z);
		}
		else targetPosition=target->position;
		// Actual4056B0 is called on the SOURCE Entity, then translated by the
		// target position. Preserve that original bound-selection policy.
		D3DXVECTOR3 low, high;
		GetBounds(low,high);
		low+=targetPosition;
		high+=targetPosition;
		LONG first, lastRow, last, firstRow;
		fog->WorldToCell(low.x,low.y,low.z,&first,&lastRow);
		fog->WorldToCell(high.x,high.y,high.z,&last,&firstRow);
		float targetHeight = float(double(high.y) + targetPosition.y - low.y);
		BYTE seen = 0;
		for (LONG row = firstRow; row <= lastRow; ++row)
			for (LONG x = first; x <= last; ++x)
			{
				if (GetLOSCell(x, row, &targetHeight)) seen = 1;
				else if (seen) return 1; //Original exits only for blocked AFTER a successful cell.
			}
		return seen ? 2 : 0;
	}
}
