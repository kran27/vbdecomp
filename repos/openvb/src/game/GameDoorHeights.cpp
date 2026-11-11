#include "GameDoor.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "model/ModelInstance.hpp"
#include "graphics/ModelInstance.hpp"
#include "graphics/AnimationInstance.hpp"
#include <math.h>

namespace F3
{
	void GameDoor::BuildFogHeights()
	{
		doorHeights.clear();
		JE::AreaMap* area = JE::g_World->GetMap(mapHandle)->areaMap;
		D3DXVECTOR3 low = position + minimum, high = position + maximum;
		LONG first, lastRow, last, firstRow;
		area->WorldToCell(low, &first, &lastRow);
		area->WorldToCell(high, &last, &firstRow);
		if (!model || !model->animation) return;
		Gfx_AnimationInstance* animation = model->animation;
		Gfx_AnimationObject* object = animation->animationObject;
		if (!object->collisionMesh || !object->meshBinding) return;
		DWORD milliseconds = static_cast<DWORD>(static_cast<__int64>(
			double(object->GetDuration(currentAnimationGroup, currentAnimationIndex, 0)) * 1000.0));
		model->UpdateAnimation(milliseconds);
		model->UpdateAfterimages(milliseconds);
		D3DXMATRIX transform = model->GetModel(0)->transform;
		for (LONG row = firstRow; row <= lastRow; ++row)
		{
			for (LONG x = first; x <= last; ++x)
			{
				float x1, z1, x2, z2;
				area->CellToWorld(x, row, &x1, &z1, 0);
				area->CellToWorld(x + 1, row + 1, &x2, &z2, 0);
				float sampleZ = z1, height = low.y;
				LONG hits = 0;
				while (z2 - 0.25 < sampleZ)
				{
					if (sampleZ < z2) sampleZ = z2;
					for (float sampleX = x1; x2 + 0.25 > sampleX; sampleX += 0.25f)
					{
						if (sampleX > x2) sampleX = x2;
						D3DXVECTOR3 origin(sampleX, high.y, sampleZ);
						D3DXVECTOR3 direction(0.00001f, -1.0f, 0.0f);
						float distance = 0;
						if (animation->RayIntersection(transform, origin, direction, distance))
						{
							float hitHeight = origin.y - static_cast<float>(fabs(distance));
							if (height <= hitHeight) height = hitHeight;
							++hits;
						}
					}
					sampleZ -= 0.25f;
				}
				if (hits > 0)
				{
					JE::AreaMapHeight cell;
					cell.first = x; cell.row = row; cell.height = height;
					doorHeights.push_back(cell);
				}
			}
		}
	}
}
