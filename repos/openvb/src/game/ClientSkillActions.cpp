#include "ClientSkillActions.hpp"
#include "GameActionUseSkill.hpp"
#include "GameEntity.hpp"
#include "entity/Entity.hpp"
#include <new>

namespace F3
{
	void QueueClientSkillSelection(const DWORD& actorID, const DWORD& targetID, const DWORD& skill)
	{
		JE::Entity* actor = JE::GetEntityPtr(actorID);
		GameEntity* target = dynamic_cast<GameEntity*>(JE::GetEntityPtr(targetID));
		if (!actor) return;
		GameActionUseSkill* action = new GameActionUseSkill;
		action->targetID = target->entityID;
		action->skill = skill;
		actor->QueueAction(action); //435EE0, no ClearActions or ownership/map test.
	}
}
