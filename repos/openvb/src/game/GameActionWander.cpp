#include "GameActionWander.hpp"
#include "entity/Entity.hpp"

namespace F3
{
	float GameActionWander::GetSpeed() { return 10.0f; }
	void GameActionWander::GetWalkAnimation(DWORD& group, DWORD& file)
	{
		owner->ResolveAnimation(299,0,&group,&file);
	}
	void GameActionWander::GetIdleAnimation(DWORD& group, DWORD& file)
	{
		owner->ResolveAnimation(416,0,&group,&file);
	}
}
