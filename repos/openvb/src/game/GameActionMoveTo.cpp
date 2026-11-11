#include "GameActionMoveTo.hpp"
#include "GameEntity.hpp"
#include "GameActionWait.hpp"
#include "GameActionOperateDoor.hpp"
#include "GameDoor.hpp"
#include "entity/Entity.hpp"
#include "entity/Map.hpp"
#include "entity/MapGrid.hpp"
#include "entity/PathManager.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include <math.h>
#include <set>
#include <stdexcept>

namespace F3
{
	GameActionMoveTo::GameActionMoveTo() {}
	GameActionMoveTo::~GameActionMoveTo() {}
	float GameActionMoveTo::GetSpeed()
	{
		GameEntity* game = dynamic_cast<GameEntity*>(owner);
		if (!game) return 0.0f;
		LONG speed = static_cast<LONG>(static_cast<__int64>(game->value94));
		return speed > 0 ? static_cast<float>(speed) : 10.0f;
	}
	void GameActionMoveTo::GetWalkAnimation(DWORD& group, DWORD& file)
	{ owner->ResolveAnimation(434,0,&group,&file); }
	void GameActionMoveTo::GetIdleAnimation(DWORD& group, DWORD& file)
	{ owner->ResolveAnimation(416,0,&group,&file); }
	void GameActionMoveTo::StartMoving()
	{
		GameEntity* game = dynamic_cast<GameEntity*>(owner);
		if (!game) return;
		game->EndAnimationNow();
		game->ClearQueuedAnimations();
		game->QueueAnimationEvent(game->value90 == 1 ? 29 : 32,0.0f);
	}
	void GameActionMoveTo::StopMoving()
	{
		GameEntity* game = dynamic_cast<GameEntity*>(owner);
		if (!game) return;
		game->EndAnimationNow();
		game->QueueAnimationEvent(1,0.0f);
	}
	void GameActionMoveTo::OnMoved()
	{
		JE::ActionMoveTo::OnMoved();
		JE::EntityGrid* grid = request->occupancy;
		if (!grid) return;
		JE::Map* map = JE::g_World->GetMap(owner->mapHandle);
		JE::PathPoint cell;
		JE::WorldToPathCell(&cell,owner->position,map);
		std::set<JE::Entity*> occupants;
		grid->CollectFootprintValues(occupants,request->owner->spatialFootprint->spans,
			cell.x,cell.y,cell.x,cell.y);
		for (std::set<JE::Entity*>::iterator entry = occupants.begin(); entry != occupants.end(); )
		{
			JE::Entity* entity = *entry;
			switch (entity->entityClass)
			{
			case 1:
				if (request->route->empty() || owner->IsVisible() != 1 || entity == owner) break;
				if (entity->moving == 1)
				{
					GameActionWait* wait = static_cast<GameActionWait*>(JE::g_World->NewAction(5));
					wait->SetDuration(1000);
					owner->QueueAction(wait);
					GameActionMoveTo* retry = static_cast<GameActionMoveTo*>(JE::g_World->NewAction(2));
					retry->SetTarget(target);
					retry->destination = destination;
					retry->request = request;
					retry->stopDistance = stopDistance;
					owner->QueueAction(retry);
					request = NULL;
					SetCompleted(1);
					return;
				}
				if (entity->IsDead()) break;
				if (entity->state != 5 && entity->state != 4) break;
				if (owner->HasMapAssociation(entity)) break;
				goto AssociateAndRetry;
			case 2: case 4:
				if (request->route->empty() || owner->IsVisible() != 1 || entity == owner
					|| owner->HasMapAssociation(entity)) break;
			AssociateAndRetry:
				owner->AddMapAssociation(entity);
				{
					GameActionMoveTo* retry = static_cast<GameActionMoveTo*>(JE::g_World->NewAction(2));
					retry->SetTarget(target);
					retry->destination = destination;
					retry->stopDistance = stopDistance;
					owner->QueueAction(retry);
				}
				SetCompleted(1);
				return;
			case 3:
				{
					GameDoor* door = dynamic_cast<GameDoor*>(entity);
					// Original repeats this held iterator for an open door. Normal
					// Door::Operate removes open doors from occupancy in normal use.
					if (door->open) continue;
					if (request->route->size() < 2) break;
					D3DXVECTOR3 initial = door->position - owner->position, candidate(0,0,0);
					const float width = static_cast<float>(fabs(door->maximum.z-door->minimum.z));
					std::list<D3DXVECTOR3>::iterator point = request->route->begin();
					for (++point; point != request->route->end(); ++point)
					{
						candidate = door->position - *point;
						D3DXVECTOR3 distance = *point - owner->position;
						if (D3DXVec3Length(&distance) >= width) break;
					}
					const float angle = door->rotation.x * 0.017453292f;
					const D3DXVECTOR3 normal(-static_cast<float>(sin(angle)),0,static_cast<float>(cos(angle)));
					const float fromSide = D3DXVec3Dot(&normal,&initial);
					const float toSide = D3DXVec3Dot(&normal,&candidate);
					if (!((fromSide < 0 && toSide > 0) || (fromSide > 0 && toSide < 0))) break;
					if (!JE::g_World->party->Contains(owner))
					{
						GameActionOperateDoor* operate = new GameActionOperateDoor;
						operate->SetDoor(door);
						owner->QueueAction(operate);
						GameActionWait* wait = static_cast<GameActionWait*>(JE::g_World->NewAction(5));
						wait->SetDuration(1000);
						owner->QueueAction(wait);
						GameActionMoveTo* retry = static_cast<GameActionMoveTo*>(JE::g_World->NewAction(2));
						retry->SetTarget(target);
						retry->destination = destination;
						retry->request = request;
						retry->stopDistance = stopDistance;
						owner->QueueAction(retry);
					}
					StopMoving();
					request = NULL;
					SetCompleted(1);
					return;
				}
			default: break;
			}
			++entry;
		}
	}
}
