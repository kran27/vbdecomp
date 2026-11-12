#include "GameWeapon.hpp"
#include "GameCreature.hpp"
#include "GameAmmo.hpp"
#include "CombatRandom.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cmath>
#include <stdexcept>

namespace F3
{
	namespace
	{
		DWORD CalledPartPenalty(DWORD part)
		{
			static const DWORD penalties[]={10,40,40,40,40,60,120,60}; //64F858.
			if (part==9) return 0;
			if (part>=8)
				throw std::logic_error("GameWeapon413121: original called-part table outside recovered domain");
			return penalties[part];
		}
		double Distance(const JE::Entity* first,const JE::Entity* second)
		{
			float x=first->position.x-second->position.x; //Original FSTP32 x difference.
			double y=double(first->position.y)-second->position.y;
			double z=double(first->position.z)-second->position.z;
			return std::sqrt(z*z+y*y+double(x)*x);
		}
		LONG ClampChance(DWORD value)
		{
			LONG chance=LONG(value);
			if (chance<0) return 0;
			return chance>95 ? 95 : chance;
		}
	}

	LONG GameWeapon::CalculateToHit(AttackModeIterator attack,JE::Entity* source,JE::Entity* victim,
		DWORD calledPart)
	{
		if (!source || !victim || currentAttackMode == AttackModeIterator())
		{
			JE::LogDebugString("GameWeapon::CalculateToHit() - Recieved a NULL pointer in critical data.");
			return 0;
		}
		//41357C reads the currently selected mode, not the argument passed on.
		return currentAttackMode->kind
			? CalculateRangedToHit(attack,source,victim,calledPart)
			: CalculateMeleeToHit(attack,source,victim,calledPart);
	}

	LONG GameWeapon::GetEffectiveRange(AttackModeIterator attack)
	{
		if ((attack != AttackModeIterator() ? attack : currentAttackMode) == AttackModeIterator()) return 0;
		if (!attack->shots) return LONG(attack->range);
		if (attack->shots>primaryAmmoLoaded || attack->values18[0]>secondaryAmmoLoaded) return 0;
		if (!primaryAmmo)
		{
			primaryAmmo=FindAmmunition(primaryCaliber,primaryModifier);
			if (!primaryAmmo)
				throw std::logic_error("GameWeapon412FBB: original fatal ammo lookup has an unwritten %d argument");
		}
		return LONG(attack->range+primaryAmmo->value968);
	}

	LONG GameWeapon::CalculateMeleeToHit(AttackModeIterator attack,JE::Entity* source,JE::Entity* victim,
		DWORD calledPart)
	{
		GameCreature* actor=dynamic_cast<GameCreature*>(source);
		GameCreature* target=dynamic_cast<GameCreature*>(victim);
		DWORD accuracy=attack->values18[2]+DWORD(actor->GetTotalSkill(1));
		if (attack->shots)
		{
			if (!primaryAmmo)
			{
				primaryAmmo=FindAmmunition(primaryCaliber,primaryModifier);
				if (!primaryAmmo)
					JE::FatalError("GameWeapon::CalculateMeleeToHit - Couldn't find ammo: caliber %d, modifier %d",
						primaryCaliber,primaryModifier);
			}
			accuracy+=primaryAmmo->value964;
		}
		DWORD defense=DWORD(target->GetDerivedAttribute(4));
		LONG range=GetEffectiveRange(attack);
		if (double(range)<Distance(source,victim)) return 0;
		defense+=CalledPartPenalty(calledPart);
		return ClampChance(accuracy-defense);
	}

	LONG GameWeapon::CalculateRangedToHit(AttackModeIterator attack,JE::Entity* source,JE::Entity* victim,
		DWORD calledPart)
	{
		GameCreature* actor=dynamic_cast<GameCreature*>(source);
		GameCreature* target=dynamic_cast<GameCreature*>(victim);
		DWORD accuracy;
		LONG range;
		if (attack->kind==2)
		{
			accuracy=attack->values18[2]+DWORD(actor->GetTotalSkill(1));
			range=actor->GetDerivedAttribute(13);
		}
		else
		{
			accuracy=attack->values18[2]+DWORD(actor->GetTotalSkill(0));
			range=GetEffectiveRange(attack);
		}
		if (attack->shots)
		{
			if (!primaryAmmo)
			{
				primaryAmmo=FindAmmunition(primaryCaliber,primaryModifier);
				if (!primaryAmmo)
					JE::FatalError("GameWeapon::CalculateRangedToHit() - Unable to locate ammo. Caliber:%d, modifier:%d",
						primaryCaliber,primaryModifier);
			}
			accuracy+=primaryAmmo->value964;
		}
		double distance=Distance(source,victim);
		LONG multiplier=0;
		LONG adjusted=LONG(DWORD(range)-10);
		if (adjusted>=0)
		{
			LONG divisor=adjusted/5;
			multiplier=200/divisor;
		}
		LONG penalty=LONG(DWORD(-2)-DWORD(static_cast<__int64>(distance*double(-0.2f))));
		if (penalty<0) penalty=0;
		DWORD distancePenalty=DWORD(multiplier)*DWORD(penalty);
		DWORD defense=DWORD(target->GetDerivedAttribute(4))+CalledPartPenalty(calledPart);
		return ClampChance(accuracy-defense-distancePenalty);
	}

	LONG GameWeapon::CalculateDamage(AttackModeIterator attack,JE::Entity* source,JE::Entity* victim,
		DWORD, DWORD calledPart)
	{
		if (attack == AttackModeIterator() || !victim || !source)
		{
			JE::LogDebugString("GameWeapon::CalculateDamage - Invalid paramaters passed.\n");
			return 0;
		}
		if (calledPart==9)
		{
			JE::LogDebugString("GameWeapon::CalculateDamage - No body part was targeted.\n");
			return 0;
		}
		GameCreature* target=dynamic_cast<GameCreature*>(victim);
		DWORD low=attack->minimumDamage,high=attack->maximumDamage;
		DWORD armorAdjustment=0;
		if (attack->shots)
		{
			if (!primaryAmmo)
			{
				primaryAmmo=FindAmmunition(primaryCaliber,primaryModifier);
				if (!primaryAmmo)
					JE::FatalError("GameWeapon::CalculateDamage() - Unable to find ammo. Caliber:%d, modifier:%d",
						primaryCaliber,primaryModifier);
			}
			DWORD minimumAddition=primaryAmmo->value952;
			armorAdjustment=primaryAmmo->value960;
			high+=primaryAmmo->value956;
			if (low+minimumAddition<high) low+=minimumAddition; //4133EB unsigned comparison.
		}
		DWORD amount=attack->values18[1]+DWORD(CombatRandomRange(LONG(low),LONG(high)));
		LONG defense;
		switch (calledPart)
		{
		case 0: case 1: case 2: case 3: case 4: case 7:
			defense=target->resistances[attack->damageType]; break;
		case 5: case 6:
			defense=target->thresholds[attack->damageType]; break;
		default:
			JE::LogDebugString("GameWeapon::CalculateDamage() - Unknown CALLEDTARGET encountered.\n");
			defense=target->resistances[attack->damageType]; break;
		}
		if (defense>0)
		{
			defense=LONG(DWORD(defense)-armorAdjustment);
			if (defense<0) defense=0;
		}
		LONG half=defense/2;
		LONG damage=LONG(amount-DWORD(half)-DWORD(CombatRandomRange(0,half)));
		return damage<0 ? 0 : damage;
	}
}
