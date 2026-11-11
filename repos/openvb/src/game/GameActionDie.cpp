#include "GameActionDie.hpp"
#include "GameEntity.hpp"

namespace F3
{
	GameActionDie::GameActionDie() : damageType(7), damageSubtype(0), animationEnd(0), direction(0) {}

	int GameActionDie::Start(DWORD time)
	{
		JE::ActionDie::Start(time); //State6 precedes game animation/RTTI work.
		kind=0;
		GameEntity* entity=dynamic_cast<GameEntity*>(owner);
		DWORD duration=0;
		if (entity)
		{
			entity->ClearQueuedAnimations();
			entity->EndAnimationNow();
			switch (damageType)
			{
			case 0:
				duration=entity->QueueAnimationEvent(damageSubtype==4 ? 45 : 46,0);
				break;
			case 1:
				duration=entity->QueueAnimationEvent(44,0);
				break;
			case 2: case 3:
				entity->QueueAnimationEvent(47,0);
				//Original falls through; the preceding death event can lock this request.
			case 5:
				duration=entity->QueueAnimationEvent(damageSubtype==1 ? 48 : damageSubtype==3 ? 51 : 52,0);
				break;
			default:
				duration=entity->QueueAnimationEvent(direction ? 18 : 17,0);
				duration+=entity->QueueAnimationEvent(43,0);
				break;
			}
		}
		animationEnd=time+duration;
		return 0;
	}

	int GameActionDie::Update(DWORD time, DWORD budget)
	{
		if (animationEnd && time>=animationEnd) completed=1;
		return JE::ActionDie::Update(time,budget);
	}
}
