#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "Entity.hpp"
#include "EntityEnvironment.hpp"
#include "World.hpp"
#include "WorldCollections.hpp"
#include "Map.hpp"
#include "MapFog.hpp"
#include <stdexcept>

namespace JE
{
	float Entity::GetViewRadius() const { return 30.0f; } //549970.

	BYTE Entity::UpdateVisibility(BYTE force) //549450, movement cache and explored writes.
	{
		IWorld* world=g_World;
		if (!world->GetParty()->Contains(this) && !visibilityOwnerFlag) return 0;
		float radius=GetViewRadius();
		if (!mapHandle) return 0;
		world=g_World;
		Map* map=world->GetMap(mapHandle);
		if (!map) return 0;
		AreaMap* area=map->areaMap;
		if (!area) return 0;
		D3DXVECTOR3 origin=position;
		LONG first,row;
		area->WorldToCell(origin.x,origin.y,origin.z,&first,&row);
		if (!force && static_cast<DWORD>(first)==value908 && static_cast<DWORD>(row)==value912) return 0;
		value908=static_cast<DWORD>(first);
		value912=static_cast<DWORD>(row);
		const float step=area->GetCellSpacing();
		float firstX=origin.x,lastX=origin.x,lastZ=origin.z;
		double firstZ=origin.z;
		if (radius>0.0f)
		{
			double remaining=radius;
			do
			{
				--first; --row;
				firstX-=step; firstZ-=step;
				lastX+=step; lastZ+=step;
				remaining-=step;
			} while (remaining>0.0f);
		}
		float z=static_cast<float>(firstZ);
		if (!(firstZ<=lastZ)) return 1;
		do
		{
			float x=firstX;
			LONG column=first;
			if (firstX<=lastX)
			{
				double dz=double(z)-origin.z;
				float rowSquared=static_cast<float>(dz*dz);
				float radiusSquared=radius*radius;
				do
				{
					double dx=double(x)-origin.x;
					if (dx*dx+rowSquared<=radiusSquared
						&& (!static_cast<Client*>(F3::g_pNetworkInstance->client)->field0x100
							|| GetLOSCell(column,row,NULL)))
					{
						map->QueueVisibleCell(entityID,column,row,1);
						map->SetExploredCell(column,row,1);
					}
					++column; x+=step;
				} while (x<=lastX);
			}
			++row; z+=step;
		} while (z<=lastZ);
		return 1;
	}

	void Entity::QueueCurrentVisibility() //549710, visible-only replay.
	{
		IWorld* world=g_World;
		if (!world->GetParty()->Contains(this) && !visibilityOwnerFlag) return;
		float radius=GetViewRadius();
		if (!mapHandle) return;
		world=g_World;
		Map* map=world->GetMap(mapHandle);
		if (!map) return;
		AreaMap* area=map->areaMap;
		if (!area) return;
		D3DXVECTOR3 origin=position;
		LONG first,row;
		area->WorldToCell(origin.x,origin.y,origin.z,&first,&row);
		const float step=area->GetCellSpacing();
		float firstX=origin.x,lastX=origin.x,lastZ=origin.z;
		double firstZ=origin.z;
		if (radius>0.0f)
		{
			double remaining=radius;
			do
			{
				--first; --row;
				firstX-=step; firstZ-=step;
				lastX+=step; lastZ+=step;
				remaining-=step;
			} while (remaining>0.0f);
		}
		float z=static_cast<float>(firstZ);
		if (!(firstZ<=lastZ)) return;
		do
		{
			float x=firstX;
			LONG column=first;
			if (firstX<=lastX)
			{
				double dz=double(z)-origin.z;
				float rowSquared=static_cast<float>(dz*dz);
				float radiusSquared=radius*radius;
				do
				{
					double dx=double(x)-origin.x;
					if (dx*dx+rowSquared<=radiusSquared
						&& (!static_cast<Client*>(F3::g_pNetworkInstance->client)->field0x100
							|| GetLOSCell(column,row,NULL)))
						map->QueueVisibleCell(entityID,column,row,1);
					++column; x+=step;
				} while (x<=lastX);
			}
			++row; z+=step;
		} while (z<=lastZ);
	}
}
