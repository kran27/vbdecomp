#include "GameWorld.hpp"
#include "GameEffectDamage.hpp"
#include <new>

namespace F3
{
	JE::Effect* GameWorld::NewEffect(DWORD type) //46FA40.
	{
		switch (type)
		{
		case 1: return new GameEffectDamage(1);
		default: return World::NewEffect(type);
		}
	}
}
