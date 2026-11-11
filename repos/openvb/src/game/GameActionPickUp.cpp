#include <winsock2.h>
#include "GameActionPickUp.hpp"
#include "GameEntity.hpp"
#include "entity/ActionMoveTo.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/World.hpp"
#include "network/EngineServer.hpp"
#include "startup/NetworkComponent.hpp"
#include <math.h>
#include <new>
#include <stdexcept>

namespace F3
{
	GameActionPickUp::GameActionPickUp() {}
	GameActionPickUp::~GameActionPickUp() {}
	LONG GameActionPickUp::StartPickupAnimation()
	{
		GameEntity* entity=dynamic_cast<GameEntity*>(owner);
		if (!entity) return 0;
		entity->EndAnimationNow();
		return entity->QueueAnimationEvent(141,0.0f);
	}
	void GameActionPickUp::OnPickupCue()
	{
		JE::Entity* item=target;
		JE::Server* server=static_cast<JE::Server*>(g_pNetworkInstance->server);
		server->RemoveEntityFromMap(item);
		item=target;
		JE::Entity* actor=owner;
		server=static_cast<JE::Server*>(g_pNetworkInstance->server);
		server->AddItemToInventory(actor,item);
	}
	int GameActionPickUp::Update(DWORD time,DWORD budget)
	{
		target=JE::GetEntityPtr(targetID);
		if (!target) { completed=1;return 0; }
		if (target->mapHandle!=owner->mapHandle) { completed=1;return 0; }
		const double x=double(owner->position.x)-target->position.x;
		const double y=double(owner->position.y)-target->position.y;
		const double z=double(owner->position.z)-target->position.z;
		if (sqrt(x*x+y*y+z*z)>=3.0)
		{
			if (!JE::g_World) throw std::logic_error("GameActionPickUp406FF4: actual World required");
			JE::ActionMoveTo* move=static_cast<JE::ActionMoveTo*>(JE::g_World->NewAction(2));
			move->SetTarget(target);
			move->stopDistance=3;
			owner->QueueAction(move);
			GameActionPickUp* retry=new(std::nothrow) GameActionPickUp; //407016 is direct construction.
			retry->SetTarget(target);
			owner->QueueAction(retry);
			completed=1;
			return 0;
		}
		if (budget<12) return 0; //407070 also gates completion; do not move this inside firstUpdate.
		int used=0;
		if (firstUpdate)
		{
			firstUpdate=0;
			FaceTarget();
			recoveryEnd=time+DWORD(StartPickupAnimation());
			used=12;
		}
		if (recoveryEnd && time>=recoveryEnd)
		{
			GameEntity* entity=dynamic_cast<GameEntity*>(owner);
			if (entity)
			{
				entity->EndAnimationNow();
				entity->QueueAnimationEvent(1,0.0f); //Shared406780.
			}
			recoveryEnd=0;
			completed=1;
		}
		return used;
	}
}
