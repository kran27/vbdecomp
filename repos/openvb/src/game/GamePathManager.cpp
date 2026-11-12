#include "GamePathManager.hpp"
#include "GameEntity.hpp"
#include "GameDoor.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "entity/SpatialFootprint.hpp"
#include <set>
#include <stdexcept>

namespace F3
{
	GamePathManager::GamePathManager() {}
	GamePathManager::~GamePathManager() {}
	LONG GamePathManager::Estimate(LONG fromX, LONG fromY, LONG toX, LONG toY)
	{
		return JE::PathManager::Estimate(fromX, fromY, toX, toY);
	}
	LONG GamePathManager::StepCost(const JE::PathPoint& from, const JE::PathPoint& to,
		JE::PathRequest* request)
	{
		LONG cost = JE::PathManager::StepCost(from, to, request);
		JE::EntityGrid* occupancy = request->occupancy;
		if (!occupancy) return cost;
		std::set<JE::Entity*> occupants;
		occupancy->CollectFootprintValues(occupants, request->owner->spatialFootprint->spans,
			request->startCell.x, request->startCell.y, from.x, from.y);
		while (!occupants.empty())
		{
			std::set<JE::Entity*>::iterator first = occupants.begin();
			JE::Entity* occupant = *first;
			switch (occupant->entityClass)
			{
			case 3:
			{
				GameDoor* door=dynamic_cast<GameDoor*>(occupant);
				if (!door->CanOperate(occupant))
					cost=static_cast<LONG>(static_cast<DWORD>(cost)+32765);
				break;
			}
			case 1:
			case 2:
			case 4:
				if (!request->owner->mapAssociations.empty()
					&& request->owner->HasMapAssociation(occupant)==1)
					cost=static_cast<LONG>(static_cast<DWORD>(cost)+32765);
				break;
			}
			occupants.erase(first);
		}
		return cost;
	}
	BYTE GamePathManager::CanTraverse(JE::PathRequest* request, D3DXVECTOR3 from, D3DXVECTOR3 to)
	{
		std::set<JE::Entity*> occupants;
		float deltaZ = to.z - from.z;
		float deltaY = to.y - from.y;
		double differenceX = double(to.x) - from.x;
		float deltaX = static_cast<float>(differenceX);
		LONG squared = static_cast<LONG>(static_cast<__int64>(differenceX * differenceX + double(deltaZ) * deltaZ));
		float samples = static_cast<float>(JE::PathIntegerSquareRoot(squared)) * 1.666f;
		if (samples > 1.0f)
		{
			float reciprocal = 1.0f / samples;
			for (LONG index = 1; double(index) < samples; ++index)
			{
				double fraction = double(index) * reciprocal;
				float x = static_cast<float>(deltaX * fraction);
				float y = static_cast<float>(deltaY * fraction);
				float z = static_cast<float>(deltaZ * fraction);
				D3DXVECTOR3 sample(x + from.x, from.y + y, z + from.z);
				JE::Map* map = JE::g_World->GetMap(request->owner->mapHandle);
				JE::PathPoint unused;
				JE::WorldToPathCell(&unused, from, map); //Original45BE24 repeats this unused conversion.
				JE::PathPoint cell;
				JE::WorldToPathCell(&cell, sample, map);
				request->occupancy->CollectFootprintValues(occupants,
					request->owner->spatialFootprint->spans, request->startCell.x,
					request->startCell.y, cell.x, cell.y);
				for (std::set<JE::Entity*>::iterator entry = occupants.begin(); entry != occupants.end(); ++entry)
				{
					JE::Entity* occupant = *entry;
					switch (occupant->entityClass)
					{
					case 1:
					case 2:
					case 4:
						if (!request->owner->mapAssociations.empty()
							&& request->owner->HasMapAssociation(occupant) == 1) return 0;
						break;
					case 3:
						if (!dynamic_cast<GameDoor*>(occupant)->open) return 0;
						break;
					}
				}
			}
		}
		return JE::PathManager::CanTraverse(request, from, to);
	}
	BYTE GamePathManager::IsBlocked(BYTE flags, JE::Entity* entity)
	{
		GameEntity* game = dynamic_cast<GameEntity*>(entity);
		LONG size = static_cast<LONG>(game->stringRefs[4]); // Original GameEntity+18; array starts+8.
		if (size <= 3 && flags >= 43) return 1;
		if (size == 4 && flags >= 32) return 1;
		if (size == 5 && flags >= 21) return 1;
		if (size >= 6 && flags >= 10) return 1;
		return 0;
	}
}
