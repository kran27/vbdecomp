#include "Map.hpp"
#include "MapFog.hpp"
#include "World.hpp"
#include "WorldCollections.hpp"

namespace JE
{
	std::list<AreaMapHeight>* Entity::GetHeightInfluence() { return NULL; }

	BYTE AreaMap::RemoveHeightList(DWORD id)
	{
		std::map<DWORD, std::list<AreaMapHeight> >::iterator found = heightLists.find(id);
		if (found == heightLists.end()) return 0;
		heightLists.erase(found);
		heightsDirty = 1;
		return 1;
	}
	BYTE AreaMap::RemoveHeightRectangle(DWORD id)
	{
		std::map<DWORD, AreaMapRectangle>::iterator found = rectangles.find(id);
		if (found == rectangles.end()) return 0;
		rectangles.erase(found);
		heightsDirty = 1;
		return 1;
	}
	BYTE Map::AddEntityHeightInfluence(Entity* entity)
	{
		if (!entity || !areaMap) return 0;
		areaMap->RemoveHeightList(entity->entityID);
		areaMap->RemoveHeightRectangle(entity->entityID);
		BYTE changed;
		if (entity->GetHeightInfluence())
		{
			const DWORD id = entity->entityID;
			std::list<AreaMapHeight>* influence = entity->GetHeightInfluence();
			changed = areaMap->SetHeightList(id, *influence);
		}
		else
		{
			const D3DXVECTOR3& position = entity->position;
			D3DXVECTOR3 low = entity->minimum;
			D3DXVECTOR3 high = entity->maximum;
			low += position;
			high += position;
			changed = areaMap->SetHeightRectangle(entity->entityID,
				low.x, low.y, low.z, high.x, high.y, high.z);
		}
		if (!changed) return 0;
		std::vector<Entity*>& members = GetWorldInstance()->GetParty()->members;
		for (std::vector<Entity*>::iterator member = members.begin(); member != members.end(); ++member)
			(*member)->UpdateVisibility(1);
		return 1;
	}
	BYTE Map::RemoveEntityHeightInfluence(Entity* entity)
	{
		if (!entity || !areaMap) return 0;
		BYTE removedList = areaMap->RemoveHeightList(entity->entityID);
		const DWORD id = entity->entityID;
		BYTE removedRectangle = areaMap->RemoveHeightRectangle(id);
		if (!(removedList | removedRectangle)) return 0;
		std::vector<Entity*>& members = GetWorldInstance()->GetParty()->members;
		for (std::vector<Entity*>::iterator member = members.begin(); member != members.end(); ++member)
			(*member)->UpdateVisibility(1);
		return 1;
	}
}
