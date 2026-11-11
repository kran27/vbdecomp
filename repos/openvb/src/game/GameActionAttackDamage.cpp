#include "GameActionAttack.hpp"
#include "GameCreature.hpp"
#include "GameWeapon.hpp"
#include "GameEffectDamage.hpp"
#include "GameScriptObjectSearch.hpp"
#include "CombatRandom.hpp"
#include "entity/World.hpp"
#include "entity/Combat.hpp"
#include "entity/EntityEnvironment.hpp"
#include "scripting/ScriptSmall.hpp"
#include "model/CommonException.hpp"
#include <algorithm>
#include <cstdio>
#include <stdexcept>

namespace F3
{
	namespace
	{

	}

	BYTE GameActionAttack::RollHit(AttackSubAttack& sub, JE::Entity* victim)
	{
		void* debug=JE::ScriptServices::GetDebugWindow();
		LONG chance=unarmed ? creature->CalculateUnarmedChance(sub.mode,victim,sub.calledShot,weapon)
			: weapon->CalculateToHit(sub.mode,owner,victim,sub.calledShot);
		LONG roll=CombatRandomRange(0,100);
		if (debug)
		{
			char message[256];
			std::sprintf(message,"Hit/Miss: Chance = %d, Random = %d",chance,roll);
			JE::ScriptServices::DebugOutput(debug,message,128,128,128);
		}
		return chance>=roll; //Inclusive: chance0 can hit on roll0.
	}

	void GameActionAttack::ChooseHitPart(AttackSubAttack& sub, JE::Entity* victim)
	{
		DWORD roll=DWORD(CombatRandomRange(1,100)); //Consumed before the RTTI query.
		GameCreature* victimCreature=dynamic_cast<GameCreature*>(victim);
		if (victimCreature) sub.calledShot=victimCreature->ChooseBodyPart(roll);
	}

	void GameActionAttack::NotifyAttacked(AttackSubAttack& sub, JE::Entity* victim)
	{
		if (!victim || victim->IsDead()) return;
		cell result=0;
		cell arguments[5]={cell(owner->entityID),cell(victim->entityID),sub.hit!=0,
			cell(sub.calledShot),0};
		owner->RunScript("OnAttacked",&result,5,arguments);
		if (!victim->RunScript("OnAttacked",&result,5,arguments) || result)
			victim->EnterCombat();
		std::vector<JE::Entity*> nearby;
		DWORD handle;
		if (FindScriptObjects(&handle,owner->mapHandle,owner->position,30.0f,owner->entityID))
		{
			const FrameLookupCaches::ObjectList* found=GetFrameObjects(handle);
			if (found)
				for (size_t i=0;i<found->size();++i)
					nearby.push_back(static_cast<JE::Entity*>((*found)[i]));
		}
		if (FindScriptObjects(&handle,victim->mapHandle,victim->position,30.0f,victim->entityID))
		{
			const FrameLookupCaches::ObjectList* found=GetFrameObjects(handle);
			if (found)
				for (size_t i=0;i<found->size();++i)
				{
					JE::Entity* actor=static_cast<JE::Entity*>((*found)[i]);
					if (std::find(nearby.begin(),nearby.end(),actor)==nearby.end()) nearby.push_back(actor);
				}
		}
		for (size_t i=0;i<nearby.size();++i)
		{
			JE::Entity* actor=nearby[i];
			if (actor!=owner && actor!=victim && !actor->IsDead()
				&& actor->RunScript("OnAttacked",&result,5,arguments) && result) actor->EnterCombat();
		}
	}

	BYTE GameActionAttack::CalculateAndQueueDamage(AttackSubAttack& sub, JE::Entity* victim)
	{
		void* debug=JE::ScriptServices::GetDebugWindow();
		LONG amount=unarmed ? creature->CalculateUnarmedDamage(sub.mode,victim,flag70,sub.calledShot,weapon)
			: weapon->CalculateDamage(sub.mode,owner,victim,flag70,sub.calledShot);
		AttackDamage next;
		next.damage=DWORD(amount);
		next.value04=sub.mode->damageType;
		next.value08=sub.mode->value0C;
		next.value0C=sub.calledShot;
		next.flag10=sub.mode->animation==1;
		next.unarmed=unarmed;
		next.ranged=ranged;
		next.attackerID=attacker->entityID;
		next.targetID=victim->entityID;
		next.weaponID=weapon ? weapon->entityID : 0;
		damage[next.targetID].push_back(next);
		char message[256];
		std::sprintf(message,"Damage for %d queued.",amount);
		if (debug) JE::ScriptServices::DebugOutput(debug,message,128,128,128);
		return 1;
	}

	void GameActionAttack::PublishPendingDamage() //403770.
	{
		for (std::map<DWORD,std::list<AttackDamage> >::iterator i=damage.begin();i!=damage.end();++i)
		{
			JE::Entity* victim=JE::GetEntityPtr(i->first);
			if (!victim) continue;
			if (!(JE::GetEntityEnvironment().NetworkMode()&1))
				throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
			if (!JE::g_World) throw std::logic_error("GameActionAttack403851: actual server World required");
			DWORD hits=0;
			BYTE multiple=0;
			for (std::list<AttackDamage>::iterator j=i->second.begin();j!=i->second.end();++j)
				if (j->flag10 && ++hits>1) { multiple=1; break; }
			for (std::list<AttackDamage>::iterator j=i->second.begin();j!=i->second.end();++j)
			{
				JE::GetEntityPtr(j->attackerID); //Original performs this otherwise unused lookup.
				JE::Effect* effect=JE::g_World->NewEffect(1);
				GameEffectDamage* next=dynamic_cast<GameEffectDamage*>(effect);
				effect->SetCreator(j->attackerID);
				effect->SetTarget(1,j->unarmed ? j->attackerID : j->weaponID);
				effect->SetTarget(2,j->targetID);
				effect->SetTarget(0,j->attackerID);
				effect->SetTarget(3,0);
				effect->SetEnabled(1);
				next->amount=j->damage;
				next->damageType=j->value04;
				next->damageSubtype=j->value08;
				next->calledPart=j->value0C;
				if (j==i->second.begin())
				{
					next->multiHit=multiple;
					next->suppressReaction=0;
				}
				else next->suppressReaction=1;
				victim->AddEffect(effect);
			}
		}
		damage.clear(); //Only reached after every effect callback completed normally.
	}
}
