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

    Action* Entity::GetCurrentAction() // 4677E0.
    {
        if (actions.empty())
        {
            return NULL;
        }
        return actions.front();
    }

    void Entity::DispatchActions(DWORD time)
    {
        // A callback from Start/Update/Finish can call ClearActions(). Never
        // destroy an Action while its Process() method is still on the stack.
        // Re-entering this dispatch for the same entity is not supported.
        if (processingAction)
        {
            return;
        }
        while (!actions.empty())
        {
            Action* action = actions.front();
            processingAction = action;
            int used;
            try
            {
                used = action->Process(time, actionBudget);
            }
            catch (...)
            {
                processingAction = NULL;
                if (actions.empty() || actions.front() != action)
                {
                    // ClearActions detached this action while Process ran.
                    delete action;
                }
                throw;
            }
            processingAction = NULL;
            actionBudget -= used;

            if (actions.empty() || actions.front() != action)
            {
                // A callback cleared or replaced the action queue. The active
                // action was intentionally not deleted by ClearActions().
                delete action;
                continue;
            }
            if (!action->IsComplete())
            {
                break;
            }
            actions.pop_front();
            delete action;
        }
    }

    void Entity::ClearActions()
    {
        // Detach first: Cancel() may invoke scripts that manipulate actions.
        // Any newly queued actions must not be deleted by this pass.
        std::deque<Action*> removed;
        removed.swap(actions);
        if (!removed.empty())
        {
            removed.front()->Cancel(); // Preserve the original first-only cancellation.
        }
        for (Action* action : removed)
        {
            if (action != processingAction)
            {
                delete action;
            }
            // DispatchActions owns the deferred deletion of processingAction.
        }
        if (!NeedsUpdate())
        {
            RemoveFromUpdateQueue();
        }
    }
} // namespace JE
