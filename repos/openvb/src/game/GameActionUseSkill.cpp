#include "GameActionUseSkill.hpp"
#include "GameEntity.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include "entity/ActionMoveTo.hpp"
#include "graphics/GfxVector3.hpp"
#include <math.h>
#include <new>

namespace F3
{
	GameActionUseSkill::GameActionUseSkill() : targetID(0), skill(0), completed(0)
	{
		//407CA0 writes starting1 via the ordinary base construction, but leaves
		//kind, owner and minimumBudget unwritten.
	}
	BYTE GameActionUseSkill::IsComplete() const { return completed; }
	void GameActionUseSkill::Cancel() {}
	int GameActionUseSkill::Start(DWORD) { return 0; }
	int GameActionUseSkill::Finish(DWORD) { return 0; }
	int GameActionUseSkill::Update(DWORD, DWORD)
	{
		JE::Entity* target = JE::GetEntityPtr(targetID);
		if (!target || target->mapHandle != owner->mapHandle)
		{
			completed = 1;
			return 0;
		}
		if (owner == target)
		{
			GameEntity* gameTarget = dynamic_cast<GameEntity*>(target);
			if (gameTarget) gameTarget->UseSkill(skill, owner);
			completed = 1;
			return 0;
		}
		double dx = double(owner->position.x) - target->position.x;
		double dy = double(owner->position.y) - target->position.y;
		double dz = double(owner->position.z) - target->position.z;
		if (sqrt(dz*dz + dy*dy + dx*dx) >= 5.0)
		{
			JE::ActionMoveTo* move = static_cast<JE::ActionMoveTo*>(JE::GetWorldInstance()->NewAction(2));
			move->SetTarget(target);
			move->stopDistance = 5;
			owner->QueueAction(move);
			GameActionUseSkill* retry = new GameActionUseSkill;
			retry->targetID = target->entityID;
			retry->skill = skill;
			owner->QueueAction(retry);
			completed = 1;
			return 0;
		}
		Gfx_Vector3f direction = {{target->position.x - owner->position.x,
			target->position.y - owner->position.y, target->position.z - owner->position.z}};
		direction.NormalizeDirection(); //407E1E calls the actual401030 value receiver.
		D3DXVECTOR3 rotation = owner->rotation;
		rotation.x = float(acos(direction.value[1] * 0.0f + direction.value[0] * 0.0f
			+ direction.value[2]) * 57.295776f);
		if (direction.value[0] < 0.0f) rotation.x = -rotation.x;
		owner->SetRotation(rotation);
		GameEntity* gameTarget = dynamic_cast<GameEntity*>(target);
		if (gameTarget) gameTarget->UseSkill(skill, owner);
		completed = 1;
		return 0;
	}
}
