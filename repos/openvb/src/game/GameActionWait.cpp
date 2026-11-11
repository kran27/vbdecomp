#include "GameActionWait.hpp"
#include "GameEntity.hpp"

namespace F3
{
	GameActionWait::GameActionWait() {}
	GameActionWait::~GameActionWait() {}
	int GameActionWait::Start(DWORD time)
	{
		JE::ActionWait::Start(time);
		GameEntity* entity = dynamic_cast<GameEntity*>(owner);
		if (entity)
		{
			entity->EndAnimationNow();
			entity->QueueAnimationEvent(9,0.0f);
			entity->QueueAnimationEvent(1,0.0f);
		}
		return 0;
	}
}
