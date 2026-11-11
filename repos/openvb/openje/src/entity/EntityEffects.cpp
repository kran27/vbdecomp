#include "Entity.hpp"
#include "Effect.hpp"

namespace JE
{
	void Entity::CompleteEffect(DWORD id)
	{
		for (std::deque<Effect*>::iterator i=effects.begin();i!=effects.end();++i)
		{
			Effect* effect=*i;
			if (effect->effectID==id)
			{
				effect->completed=1;
				return;
			}
		}
	}
	void Entity::AddEffect(Effect* effect) //5506C0, incidental return is unused.
	{
		effect->SetOwner(entityID);
		effect->InitializeTree();
		effect->BeginTree();
		effect->ActivateTree();
		if (effect->IsComplete()) effect->Destroy();
		else
		{
			effects.push_back(effect);
			AddToUpdateQueue();
		}
	}
	void Entity::UpdateEffects() //54C597..54C641, before messages/scripts/actions.
	{
		for (std::deque<Effect*>::size_type i=0;i!=effects.size();)
		{
			Effect* effect=effects[i];
			effect->UpdateTree();
			if (effect->IsComplete())
			{
				effects.erase(effects.begin()+i);
				effect->Destroy();
				associationsDirty=1;
			}
			else ++i;
		}
	}
	void Entity::RefreshEffects() //54AA2D..54AA80, after virtual association callback.
	{
		for (std::deque<Effect*>::size_type i=0;i!=effects.size();++i)
			effects[i]->RefreshTree();
	}
	void Entity::ClearEffects() //54C390, erase before Destroy; scheduling after loop.
	{
		std::deque<Effect*>::iterator i=effects.begin();
		while (i!=effects.end())
		{
			Effect* effect=*i;
			i=effects.erase(i);
			effect->Destroy();
		}
		if (!NeedsUpdate()) RemoveFromUpdateQueue();
		associationsDirty=1;
	}
}
