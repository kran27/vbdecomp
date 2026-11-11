#include <winsock2.h>
#include "ActionPickUp.hpp"
#include "ActionMoveTo.hpp"
#include "Entity.hpp"
#include "EntityEnvironment.hpp"
#include "World.hpp"
#include "network/EngineServer.hpp"
#include <math.h>
#include <stdexcept>

namespace
{
	double PickupDistance(const D3DXVECTOR3& a,const D3DXVECTOR3& b)
	{
		const double x=double(a.x)-b.x,y=double(a.y)-b.y,z=double(a.z)-b.z;
		return sqrt(x*x+y*y+z*z);
	}
	DWORD PlayPhase(JE::Entity* owner,DWORD time,DWORD group,DWORD file)
	{
		if (group==DWORD(-1)) return time;
		owner->PlayAnimation(group,file,1,0,0);
		return time-DWORD(static_cast<__int64>(double(owner->GetAnimationDuration())*-1000.0f));
	}
}

namespace JE
{
	ActionPickUp::ActionPickUp() : target(NULL),targetID(0),approachEnd(0),pickupEnd(0),
		recoveryEnd(0),firstUpdate(1),completed(0)
	{
		kind=1; //563544; minimumBudget stays constructor-unwritten as in the original.
	}
	ActionPickUp::~ActionPickUp() {}
	BYTE ActionPickUp::IsComplete() const { return completed; }
	void ActionPickUp::Cancel() {}
	int ActionPickUp::Start(DWORD) { return 0; }
	int ActionPickUp::Finish(DWORD) { return 0; }
	void ActionPickUp::GetApproachAnimation(DWORD& group,DWORD& file) { group=DWORD(-1);file=0; }
	void ActionPickUp::GetPickupAnimation(DWORD& group,DWORD& file) { group=DWORD(-1);file=0; }
	void ActionPickUp::GetRecoveryAnimation(DWORD& group,DWORD& file) { group=DWORD(-1);file=0; }
	void ActionPickUp::GetIdleAnimation(DWORD& group,DWORD& file) { group=DWORD(-1);file=0; }
	void ActionPickUp::SetTarget(Entity* entity)
	{
		target=entity;
		if (entity) targetID=entity->entityID;
	}
	void ActionPickUp::FaceTarget()
	{
		D3DXVECTOR3 direction=target->position-owner->position;
		//401030 is the game's ordinary CRT-math normalization, including division
		//by zero for coincident positions. Do not invent a facing fallback.
		const double inverse=1.0/sqrt(double(direction.x)*direction.x+
			double(direction.y)*direction.y+double(direction.z)*direction.z);
		direction.x=float(inverse*direction.x);
		direction.y=float(inverse*direction.y);
		direction.z=float(inverse*direction.z);
		D3DXVECTOR3 rotation=owner->rotation;
		rotation.x=float(acos(double(direction.y)*0.0f+double(direction.x)*0.0f+direction.z)*57.295776f);
		if (direction.x<0.0f) rotation.x=-rotation.x;
		owner->SetRotation(rotation);
	}
	int ActionPickUp::Update(DWORD time,DWORD)
	{
		target=GetEntityPtr(targetID);
		if (!target) { completed=1;return 0; }
		if (target->mapHandle!=owner->mapHandle) { completed=1;return 0; }
		if (PickupDistance(owner->position,target->position)>=3.0)
		{
			IWorld* world = g_World;
			ActionMoveTo* move=static_cast<ActionMoveTo*>(world->NewAction(2));
			move->SetTarget(target);
			move->stopDistance=3;
			owner->QueueAction(move);
			world = g_World;
			ActionPickUp* retry=static_cast<ActionPickUp*>(world->NewAction(3));
			retry->SetTarget(target);
			owner->QueueAction(retry); //56365B preserves ECX across406A30; it is this actual retry.
			completed=1;
			return 0;
		}
		DWORD group,file;
		if (firstUpdate)
		{
			firstUpdate=0;
			FaceTarget();
			GetApproachAnimation(group,file);
			approachEnd=PlayPhase(owner,time,group,file);
		}
		if (approachEnd && time>=approachEnd)
		{
			GetPickupAnimation(group,file);
			pickupEnd=PlayPhase(owner,time,group,file);
			approachEnd=0;
		}
		if (pickupEnd && time>=pickupEnd)
		{
			GetEntityEnvironment().GetServer()->RemoveEntityFromMap(target);
			GetEntityEnvironment().GetServer()->AddItemToInventory(owner,target);
			GetRecoveryAnimation(group,file);
			recoveryEnd=PlayPhase(owner,time,group,file);
			pickupEnd=0;
		}
		if (recoveryEnd && time>=recoveryEnd)
		{
			GetIdleAnimation(group,file);
			if (group!=DWORD(-1)) owner->PlayAnimation(group,file,1,0,0);
			recoveryEnd=0;
			completed=1;
		}
		return 0;
	}
}
