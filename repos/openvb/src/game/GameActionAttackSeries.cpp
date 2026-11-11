#include "GameActionAttack.hpp"
#include "GameEntity.hpp"
#include "GameWeapon.hpp"
#include "AttackMode.hpp"
#include "entity/Entity.hpp"
#include "graphics/VfxValueOwners.hpp"
#include <string.h>
#include <stdexcept>

namespace F3
{
	BYTE GameActionAttack::BuildSubAttacks() //403300.
	{
		FaceTarget();
		if (mode->kind == 3) return BuildUnarmedSubAttacks();
		return BuildWeaponSubAttacks();
	}

	BYTE GameActionAttack::BuildUnarmedSubAttacks() //402D80.
	{
		AttackSubAttack next;
		next.hit=0;
		next.animation=mode->pose;
		next.rotation=owner->rotation;
		next.calledShot=calledShotPart;
		next.mode=mode;
		next.playAnimation=1;
		next.waitForAnimationEnd=1;
		subAttacks.push_back(next);
		return 1;
	}

	BYTE GameActionAttack::BuildWeaponSubAttacks() //402B70.
	{
		if (mode->animation==1 && !mode->flag66)
		{
			//402C05 captures the owner before the actual52C260 array constructors.
			const D3DXVECTOR3 initial=owner->rotation;
			JE::VFX_Rotation rotations[6];
			for (DWORD index=0;index<6;++index)
				memcpy(rotations[index].values,&initial,sizeof(initial));
			rotations[2].values[0]=initial.x+5.0f;
			rotations[3].values[0]=initial.x-5.0f;
			rotations[4].values[0]=initial.x+10.0f;
			rotations[5].values[0]=initial.x-10.0f;
			//Native signed count is reloaded after each list publication.
			for (LONG i=0;i<static_cast<LONG>(mode->shots);++i)
			{
				AttackSubAttack next;
				next.hit=0;
				next.animation=mode->pose;
				memcpy(&next.rotation,rotations[i%6].values,sizeof(next.rotation));
				next.calledShot=calledShotPart;
				next.mode=mode;
				next.playAnimation=i==0;
				next.waitForAnimationEnd=0;
				subAttacks.push_back(next);
			}
			return 1;
		}
		AttackSubAttack next;
		next.hit=0;
		next.animation=mode->pose;
		next.rotation=owner->rotation;
		next.calledShot=calledShotPart;
		next.mode=mode;
		next.playAnimation=1;
		next.waitForAnimationEnd=1;
		subAttacks.push_back(next);
		return 1;
	}

	BYTE GameActionAttack::ResolveSubAttackTargets() //402E00.
	{
		pendingTargets.clear();
		JE::Entity* entity=JE::GetEntityPtr(targetID);
		if (currentSubAttack.mode->kind==3)
		{
			if (entity) pendingTargets.push_back(entity);
		}
		else
			weapon->ResolveTargets(owner,entity,currentSubAttack.mode,currentSubAttack,
				pointTarget,targetPosition,pendingTargets);
		return 1;
	}
}
