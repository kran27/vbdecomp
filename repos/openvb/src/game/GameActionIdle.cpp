#include "GameActionIdle.hpp"
#include "GameEntity.hpp"

namespace F3
{
	GameActionIdle::GameActionIdle() : percent(0), endCurrent(1)
	{
		kind = 1;
		// Original base leaves owner, unknown+8 and minimumBudget+C unwritten.
	}
	int GameActionIdle::Start(DWORD time)
	{
		const int result = JE::ActionIdle::Start(time);
		GameEntity* entity = dynamic_cast<GameEntity*>(owner);
		if (entity)
		{
			if (endCurrent) entity->EndAnimationNow();
			entity->QueueAnimationEvent(9, 0.0f);
			entity->QueueAnimationEvent(1, percent);
		}
		return result; // Native retains the base result across animation callbacks.
	}
}
