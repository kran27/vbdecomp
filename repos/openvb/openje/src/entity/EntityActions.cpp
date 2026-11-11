#include "Entity.hpp"
#include "Action.hpp"

namespace JE
{
	void Entity::QueueAction(Action* action)
	{
		actions.push_back(action); // Original550720 publishes before setting its owner.
		action->owner = this;
		AddToUpdateQueue();
	}

	Action* Entity::GetCurrentAction() //4677E0.
	{
		if (actions.empty()) return NULL;
		return actions.front();
	}

	void Entity::DispatchActions(DWORD time)
	{
		// Dispatch/completion factor of54C77C..54C821. The original incomplete-
		// action budget test54C831 and scheduling tail are deliberately outside
		// this entry: ActionAnim leaves minimumBudget unwritten. No invented cost.
		while (!actions.empty())
		{
			Action* action = actions.front();
			actionBudget -= action->Process(time, actionBudget);
			if (!action->IsComplete()) break;
			actions.pop_front();
			delete action;
		}
	}

	void Entity::ClearActions()
	{
		std::deque<Action*>::iterator i = actions.begin();
		if (i != actions.end())
		{
			(*i)->Cancel(); // Original cancels only the first action.
			while (i != actions.end())
			{
				Action* action = *i;
				i = actions.erase(i);
				delete action;
			}
		}
		if (!NeedsUpdate()) RemoveFromUpdateQueue();
	}
}
