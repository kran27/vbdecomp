#include "Entity.hpp"
#include "World.hpp"
#include "Action.hpp"
#include "scripting/ScriptSmall.hpp"
#include <stdexcept>

namespace JE
{
	void Entity::Update(DWORD time)
	{
		const DWORD cutscene = g_World->GetCutsceneState(); // Captured before effect/message callbacks.
		UpdateEffects();
		DispatchMessage(); //Original one-front callback precedes the ordinary ScriptsEnabled gate.
		if (ScriptServices::ScriptsEnabled())
		{
			cell result;
			cell arguments[2];
			arguments[0] = static_cast<cell>(time);
			if (cutscene)
			{
				arguments[1] = static_cast<cell>(g_World->GetCutsceneID());
				switch (cutscene)
				{
				case 1: RunScript("OnCutsceneStart", &result, 2, arguments); break;
				case 2: RunScript("OnCutscene", &result, 2, arguments); break;
				case 3: RunScript("OnCutsceneEnd", &result, 2, arguments); break;
				}
			}
			else RunScript("OnUpdate", &result, 1, arguments);
		}
		DispatchActions(time);
		Action* current = actions.empty() ? NULL : actions.front();
		// Original54C831 reads this member even though the recovered action
		// constructors leave it unwritten. Retain that quirk; the scheduling flag
		// in this indeterminate domain is not a validated behavior claim.
		if (current && actionBudget < current->minimumBudget) actionBlocked = 1;
		if (!actionBlocked && !current && actionBudget)
		{
			if (actionAlternating)
			{
				actionAlternating = 0;
				actionBlocked = 1;
			}
			else actionAlternating = 1;
		}
		if (!actionAlternating && !NeedsUpdate()) RemoveFromUpdateQueue();
		UpdateAnimationQueue();
	}
}
