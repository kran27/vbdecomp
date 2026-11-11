#include "GameActionAttack.hpp"
#include "GameCreature.hpp"
#include "GameInventory.hpp"
#include "GameWeapon.hpp"
#include "GameUsableObject.hpp"
#include "GameActionMoveTo.hpp"
#include "GameActionIdle.hpp"
#include "GameActionReload.hpp"
#include "entity/Entity.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/World.hpp"
#include "entity/Combat.hpp"
#include "entity/VisualEffect.hpp"
#include "graphics/VfxVeg.hpp"
#include "scripting/ScriptSmall.hpp"
#include "model/CommonException.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <math.h>
#include <new>
#include <cstdio>
#include <stdexcept>

namespace F3
{
	namespace
	{
		void Debug(void* window, const char* message, BYTE blue=128)
		{
			if (window) JE::ScriptServices::DebugOutput(window,message,128,128,blue);
		}

		JE::VisualEffect* FindEffect(JE::Entity* owner, DWORD id) //54C140.
		{
			std::map<DWORD,JE::VisualEffect*>::iterator found=owner->effectIDs.find(id);
			return found==owner->effectIDs.end() ? NULL : found->second;
		}
	}

	AttackSubAttack& AttackSubAttack::operator=(const AttackSubAttack& source)
	{
		hit=source.hit;
		animation=source.animation;
		rotation.x=source.rotation.x;
		rotation.y=source.rotation.y;
		rotation.z=source.rotation.z;
		calledShot=source.calledShot;
		mode=source.mode;
		playAnimation=source.playAnimation;
		waitForAnimationEnd=source.waitForAnimationEnd;
		return *this;
	}

	GameActionAttack::GameActionAttack() : attacker(NULL), creature(NULL), inventory(NULL),
		weaponID(0), weapon(NULL), value2C(0), value30(0), mode(), targetID(0),
		target(NULL), targetCreature(NULL), targetInventory(NULL), pointTarget(0),
		targetPosition(0,0,0), completed(0), calledShot(0), ammoDeducted(0),
		preparePending(1), flag5D(0), flag5E(0), prepareDeadline(0), doSubAttack(0),
		receivedDamageCue(0), subAttackDeadline(0), rotateToTarget(1), resolvePending(1),
		cancelPending(0), autoMove(1), flag70(0), flag71(0), ranged(0), unarmed(0),
		range(0), calledShotPart(9), subAttackActive(0), effectID(0)
	{
		kind=1;
		//403E40 leaves owner/minimumBudget, reloadRequested(+5B), and the
		//remaining current-subattack fields unwritten; do not zero the object.
		currentSubAttack.rotation.x=0; //Original+90/+94/+98.
		currentSubAttack.rotation.y=0;
		currentSubAttack.rotation.z=0;
		currentSubAttack.mode=AttackModeIterator(); //Original+A0.
	}
	GameActionAttack::~GameActionAttack() {} //403C70: containers own nodes, not borrowed targets/modes.
	BYTE GameActionAttack::IsComplete() const { return completed; }

	void GameActionAttack::SetTarget(JE::Entity* value)
	{
		DWORD id=value ? value->entityID : 0;
		if (targetID && targetID!=id)
		{
			completed=1;
			GameActionAttack* next=new(std::nothrow) GameActionAttack;
			next->SetTarget(value);
			next->calledShotPart=calledShotPart;
			next->pointTarget=pointTarget;
			if (pointTarget) next->targetPosition=targetPosition;
			owner->QueueAction(next);
		}
		targetID=id;
	}
	GameActionAttack* GameActionAttack::CloneRequest()
	{
		GameActionAttack* next=new(std::nothrow) GameActionAttack;
		next->SetTarget(JE::GetEntityPtr(targetID));
		next->reloadRequested=reloadRequested; //Original copied raw request byte+5B.
		next->calledShotPart=calledShotPart;
		next->pointTarget=pointTarget;
		if (pointTarget) next->targetPosition=targetPosition;
		return next;
	}
	int GameActionAttack::Start(DWORD)
	{
		if (!targetID && !pointTarget)
			JE::FatalError("GameActionAttack::OnCreate() - No target was specified for the attack.");
		if (owner && !owner->IsDead())
		{
			JE::Entity* value=pointTarget ? NULL : JE::GetEntityPtr(targetID);
			if (pointTarget || (value && !value->IsDead())) { owner->EnterCombat(); return 0; }
		}
		completed=1;
		return 0;
	}
	void GameActionAttack::Cancel()
	{
		Debug(JE::ScriptServices::GetDebugWindow(),"Current attack canceled.");
		if (effectID && weapon)
		{
			JE::VisualEffect* effect=FindEffect(weapon,effectID);
			if (effect && effect->instance) effect->instance->Stop(); //Actual48DCC0, not removal.
		}
		GameEntity* entity=dynamic_cast<GameEntity*>(owner);
		if (entity) entity->QueueAnimationEvent(9,0);
	}
	int GameActionAttack::Finish(DWORD)
	{
		Debug(JE::ScriptServices::GetDebugWindow(),"Current attack has finished.",0);
		return 0;
	}
	bool GameActionAttack::CanFinish() const
	{
		return !(effectID && weapon && FindEffect(weapon,effectID)); //401410.
	}
	void GameActionAttack::OnReleaseCue() //4017D0; the original cue value is unused.
	{
		if (mode == AttackModeIterator() || mode->effectName.empty()) return;
		if (unarmed)
		{
			if (!owner) return;
			effectID=JE::AllocateVisualEffectID();
			owner->AttachVisualEffect(mode->effectName.c_str(),effectID,1);
		}
		else if (weapon)
		{
			effectID=JE::AllocateVisualEffectID();
			weapon->AttachVisualEffect(mode->effectName.c_str(),effectID,1);
		}
	}
	void GameActionAttack::Complete(BYTE queueIdle)
	{
		owner->LeaveCombat(); //551C80, actual no-argument owner boundary.
		if (queueIdle)
		{
			JE::IWorld* world=JE::GetWorldInstance();
			GameActionIdle* idle=static_cast<GameActionIdle*>(world->NewAction(1));
			idle->endCurrent=0;
			owner->QueueAction(idle);
		}
		completed=1;
	}
	void GameActionAttack::CancelWhenReady(BYTE queueIdle)
	{
		if (CanFinish()) Complete(queueIdle);
		else cancelPending=1;
	}
	bool GameActionAttack::NeedsAmmo() const
	{
		return weapon && mode != AttackModeIterator() && ((mode->shots && mode->shots>weapon->primaryAmmoLoaded)
			|| (mode->values18[0] && mode->values18[0]>weapon->secondaryAmmoLoaded));
	}
	BYTE GameActionAttack::ResolveAttack()
	{
		void* debug=JE::ScriptServices::GetDebugWindow();
		if (!creature->ResolveWeaponAndAttackMode(&weapon,&mode,&unarmed))
		{
			CancelWhenReady(1);
			Debug(debug,"GameActionAttack - ResolveWeaponAndAttackMode failed.");
			return range!=0; //402641 return is nonzero range, confirmed from assembly.
		}
		switch (mode->kind)
		{
		case 0: case 3:
			range=static_cast<LONG>(mode->range);
			if (debug)
			{
				char message[512];
				std::sprintf(message,"Resolved as melee or unarmed with a range of %d",static_cast<int>(range));
				Debug(debug,message);
			}
			break;
		case 1: case 2:
			ranged=1; range=-1;
			Debug(debug,"Resolved as ranged so no auto-move will be queued no matter what.");
			break;
		default:
			CancelWhenReady(1);
			Debug(debug,"GameActionAttack - Invalid attack type found in the attack mode. Cancelling.");
			return range!=0;
		}
		if (mode->calledShot)
		{
			if (calledShotPart==9)
			{
				Debug(debug,"GameActionAttack - Attack mode is called shot, but SetCalledShot has not been called.");
				return range!=0;
			}
			calledShot=1;
		}
		if (weapon) weaponID=weapon->entityID;
		return range!=0;
	}
	void GameActionAttack::FaceTarget()
	{
		if (!rotateToTarget) return;
		D3DXVECTOR3 origin=owner->position;
		JE::Entity* targetEntity=JE::GetEntityPtr(targetID);
		if (!pointTarget && !targetEntity) return;
		D3DXVECTOR3 destination=pointTarget ? targetPosition : targetEntity->position;
		D3DXVECTOR3 rotation=owner->rotation;
		float dx=destination.x-origin.x;
		double dy=double(destination.y)-origin.y, dz=double(destination.z)-origin.z;
		double inverseTargetDistance=1.0/sqrt(dz*dz+dy*dy+double(dx)*dx);
		float x=static_cast<float>(dx*inverseTargetDistance);
		float y=static_cast<float>(dy*inverseTargetDistance);
		float z=static_cast<float>(dz*inverseTargetDistance);
		rotation.x=static_cast<float>(acos(y*0.0+double(x)*0.0+z)*57.2957763671875f);
		if (x<0) rotation.x=-rotation.x;
		owner->SetRotation(rotation);
	}
	BYTE GameActionAttack::PrepareAttack()
	{
		FaceTarget();
		if (!NeedsAmmo()) return 1;
		Debug(JE::ScriptServices::GetDebugWindow(),"GameActionAttack::PrepareAttack() - Insufficient ammo to do this attack.");
		CancelWhenReady(1);
		return 0;
	}
	int GameActionAttack::Update(DWORD time, DWORD budget)
	{
		void* debug=JE::ScriptServices::GetDebugWindow();
		if (cancelPending) { if (CanFinish()) Complete(1); return 0; }
		attacker=dynamic_cast<GameEntity*>(owner);
		if (!attacker || attacker->IsDead()) { CancelWhenReady(1); return 0; }
		inventory=dynamic_cast<GameInventory*>(attacker->inventory);
		if (!inventory) { CancelWhenReady(1); return 0; }
		creature=dynamic_cast<GameCreature*>(attacker);
		if (!creature) { CancelWhenReady(1); return 0; }
		JE::Entity* targetEntity=JE::GetEntityPtr(targetID);
		if (!pointTarget)
		{
			if (!targetEntity) { CancelWhenReady(1); return 0; }
			target=dynamic_cast<GameEntity*>(targetEntity);
			if (!target) { CancelWhenReady(1); return 0; }
		}
		if (weaponID) weapon=dynamic_cast<GameWeapon*>(JE::GetEntityPtr(weaponID));
		if (!pointTarget)
		{
			if (targetEntity->mapHandle!=owner->mapHandle)
			{
				CancelWhenReady(1); Debug(debug,"Target is not on the same map. Cancelling."); return 0;
			}
			if (targetEntity->IsDead())
			{
				CancelWhenReady(1); Debug(debug,"GameActionAttack::OnServerUpdate() - Target entity is already dead."); return 0;
			}
		}
		//At fresh entry weapon isNULL, so the original ammo predicate is false
		//independently of constructor-unwritten reloadRequested.
		if (weapon && reloadRequested && NeedsAmmo())
		{
			CancelWhenReady(1); //404514; this branch cancels rather than queuing reload.
			Debug(debug,"Out of ammo for this weapon.");
			return 0;
		}
		reloadRequested=0;
		if (resolvePending)
		{
			if (!ResolveAttack()) { CancelWhenReady(1); return 0; }
			resolvePending=0;
		}
		if (!pointTarget)
		{
			GameUsableObject* usable=mode->flag64 ? dynamic_cast<GameUsableObject*>(target) : NULL;
			targetCreature=dynamic_cast<GameCreature*>(target);
			if (!targetCreature && !usable) { CancelWhenReady(1); return 0; }
			targetInventory=dynamic_cast<GameInventory*>(target->inventory);
			if (!targetInventory && !usable) { CancelWhenReady(1); return 0; }
		}
		D3DXVECTOR3 origin=owner->position;
		D3DXVECTOR3 destination=pointTarget ? targetPosition : targetEntity->position;
		double dx=double(origin.x)-destination.x,dy=double(origin.y)-destination.y,dz=double(origin.z)-destination.z;
		double distance=sqrt(dx*dx+dy*dy+dz*dz);
		if (autoMove && ((range>0 && distance>range)
			|| !(pointTarget ? owner->GetLOSPoint(targetPosition.x, targetPosition.y, targetPosition.z) : owner->GetLOS(targetEntity))))
		{
			GameActionMoveTo* move=new(std::nothrow) GameActionMoveTo;
			if (pointTarget) move->destination=targetPosition;
			else move->SetTarget(targetEntity);
			move->stopDistance=static_cast<DWORD>(range);
			owner->QueueAction(move);
			owner->QueueAction(CloneRequest());
			CancelWhenReady(0);
			Debug(debug,"Moving closer to target.");
			return 0;
		}
		if (preparePending)
		{
			Debug(debug,"Preparing attack.");
			preparePending=0;
			if (!PrepareAttack())
			{
				owner->QueueAction(new(std::nothrow) GameActionReload);
				return 0;
			}
			prepareDeadline=time+attacker->QueueAnimationEvent(7,0);
		}
		DWORD used=0;
		if (prepareDeadline && prepareDeadline<=time && budget>=mode->actionPoints)
		{
			prepareDeadline=0;
			if (!BuildSubAttacks()) return 0;
			used=mode->actionPoints;
			doSubAttack=1;
		}
		if (doSubAttack)
		{
			if (subAttacks.empty())
				JE::FatalError("GameActionAttack::OnServerUpdate() - m_subAttackSeries empty after m_bDoSubAttack true.");
			doSubAttack=0;
			currentSubAttack=subAttacks.front(); //401240, original scalar value copy.
			subAttacks.pop_front();
			subAttackActive=1;
			subAttackDeadline=currentSubAttack.playAnimation
				? time+attacker->QueueAnimationEvent(currentSubAttack.animation,0) : time;
		}
		FaceTarget(); //Actual4012E0 is called after the subattack queue/deadline update.
		if (subAttackActive && (receivedDamageCue || (subAttackDeadline && subAttackDeadline<=time)))
		{
			subAttackActive=0;
			if (!currentSubAttack.waitForAnimationEnd) subAttackDeadline=time;
			if (!ResolveSubAttackTargets()) return static_cast<int>(used);
			while (!pendingTargets.empty())
			{
				JE::Entity* pending=pendingTargets.front();
				pendingTargets.pop_front(); //40496B unlinks before hit/chance evaluation.
				currentSubAttack.hit=RollHit(currentSubAttack,pending);
				if (currentSubAttack.calledShot==9) ChooseHitPart(currentSubAttack,pending);
				NotifyAttacked(currentSubAttack,pending);
				if (currentSubAttack.hit) CalculateAndQueueDamage(currentSubAttack,pending);
				else if (debug) JE::ScriptServices::DebugOutput(debug,"SubAttack MISSED",128,0,0);
				if (!ammoDeducted)
				{
					if (mode->shots) weapon->DeductPrimaryAmmo(mode->shots);
					if (mode->values18[0]) weapon->DeductSecondaryAmmo(mode->values18[0]);
					ammoDeducted=1;
				}
			}
			if (mode->animation!=1 || subAttacks.empty()) PublishPendingDamage();
		}
		if (subAttackDeadline && subAttackDeadline<=time)
		{
			subAttackDeadline=0;
			if (!subAttacks.empty()) { doSubAttack=1; return static_cast<int>(used); }
			if (!pointTarget && target->IsDead()) { CancelWhenReady(1); return static_cast<int>(used); }
			if (subAttacks.empty())
			{
				completed=1;
				if (!pointTarget)
				{
					GameActionAttack* next=new(std::nothrow) GameActionAttack;
					next->SetTarget(targetEntity);
					if (calledShot) next->calledShotPart=calledShotPart;
					owner->QueueAction(next);
				}
			}
		}
		return static_cast<int>(used);
	}
}
