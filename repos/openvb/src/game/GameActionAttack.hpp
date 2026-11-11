#pragma once

#include "entity/Action.hpp"
#include "AttackMode.hpp"
#include <d3dx8math.h>
#include <list>
#include <map>

namespace F3
{
	class GameEntity;
	class GameCreature;
	class GameInventory;
	class GameWeapon;
	struct AttackSubAttack
	{
		AttackSubAttack& operator=(const AttackSubAttack& source); //401240.
		BYTE hit;
		DWORD animation;
		D3DXVECTOR3 rotation;
		DWORD calledShot;
		AttackModeIterator mode;
		BYTE playAnimation, waitForAnimationEnd;
	};
	struct AttackDamage
	{
		DWORD damage, value04, value08, value0C;
		BYTE flag10, unarmed, ranged;
		DWORD attackerID, targetID, weaponID;
	};

	class GameActionAttack : public JE::Action
	{
	public:
		GameActionAttack(); //403E40, real list/map owners and actual scalar stores.
		virtual ~GameActionAttack(); //403C70, normal container lifetime only.
		virtual BYTE IsComplete() const; //403D50.
		virtual void Cancel(); //402080, does not set completed.
		virtual int Start(DWORD time); //4011D0, target/dead checks and actual Combat.AddEntity.
		virtual int Update(DWORD time, DWORD budget); //404320, connected preparation/movement prefix.
		virtual int Finish(DWORD time); //4021B0.
		void SetTarget(JE::Entity* target); //403FA0, changed target queues a successor.
		GameActionAttack* CloneRequest(); //404270, preserves request fields only.
		void OnReleaseCue(); //4017D0, actual attack-mode VFX attachment; cue argument unused.

		GameEntity* attacker;
		GameCreature* creature;
		GameInventory* inventory;
		DWORD weaponID;
		GameWeapon* weapon;
		DWORD value2C, value30;
		AttackModeIterator mode;
		DWORD targetID;
		GameEntity* target;
		GameCreature* targetCreature;
		GameInventory* targetInventory;
		BYTE pointTarget;
		D3DXVECTOR3 targetPosition;
		BYTE completed, calledShot, ammoDeducted, reloadRequested;
		BYTE preparePending, flag5D, flag5E;
		DWORD prepareDeadline;
		BYTE doSubAttack, receivedDamageCue;
		DWORD subAttackDeadline;
		BYTE rotateToTarget, resolvePending, cancelPending, autoMove;
		BYTE flag70, flag71, ranged, unarmed;
		LONG range;
		DWORD calledShotPart;
		std::list<AttackSubAttack> subAttacks;
		AttackSubAttack currentSubAttack;
		BYTE subAttackActive;
		std::list<JE::Entity*> pendingTargets;
		DWORD effectID;
		std::map<DWORD,std::list<AttackDamage> > damage;
	private:
		BYTE ResolveAttack(); //4023C0.
		bool NeedsAmmo() const; //401790.
		bool CanFinish() const; //401410.
		void Complete(BYTE queueIdle); //401890.
		void CancelWhenReady(BYTE queueIdle); //401A10.
		void FaceTarget(); //4012E0.
		BYTE PrepareAttack(); //402680.
		BYTE BuildSubAttacks(); //403300, facing then actual child dispatch.
		BYTE BuildUnarmedSubAttacks(); //402D80, ordinary thiscall/RET.
		BYTE BuildWeaponSubAttacks(); //402B70, ordinary thiscall/RET.
		BYTE ResolveSubAttackTargets(); //402E00, actual unarmed target list; weapon owner remains addressed.
		BYTE RollHit(AttackSubAttack& sub, JE::Entity* victim); //402260.
		void ChooseHitPart(AttackSubAttack& sub, JE::Entity* victim); //401280.
		void NotifyAttacked(AttackSubAttack& sub, JE::Entity* victim); //403320.
		BYTE CalculateAndQueueDamage(AttackSubAttack& sub, JE::Entity* victim); //404060.
		void PublishPendingDamage(); //403770, empty normal map branch; live damage effects addressed.
		GameActionAttack(const GameActionAttack&);
		GameActionAttack& operator=(const GameActionAttack&);
	};
}
