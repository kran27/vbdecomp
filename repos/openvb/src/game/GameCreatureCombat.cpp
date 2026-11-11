#include "GameCreature.hpp"
#include "GameWeapon.hpp"
#include "GameUsableObject.hpp"
#include "CombatRandom.hpp"
#include "startup/Random.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace F3
{
	LONG CombatRandomRange(LONG first, LONG last)
	{
		if (first==last) return first;
		if (last<first) { LONG swap=first; first=last; last=swap; }
		DWORD span=DWORD(last)-DWORD(first)+1;
		if (!span) throw std::logic_error("RandomRange48CD00: original divide by zero range");
		return LONG(DWORD(first)+RandomNext()%span);
	}

	DWORD GameCreature::ChooseBodyPart(DWORD roll) const
	{
		const DWORD through1 = DWORD(uncalledChances[0]) + DWORD(uncalledChances[1]);
		const DWORD through2 = through1 + DWORD(uncalledChances[2]);
		const DWORD through3 = through2 + DWORD(uncalledChances[3]);
		const DWORD through4 = through3 + DWORD(uncalledChances[4]);
		const DWORD through5 = through4 + DWORD(uncalledChances[5]);
		const DWORD through6 = through5 + DWORD(uncalledChances[6]);
		if (roll <= DWORD(uncalledChances[0])) return 0;
		if (roll <= through1) return 1;
		if (roll <= through2) return 2;
		if (roll <= through3) return 3;
		if (roll <= through4) return 4;
		if (roll <= through5) return 5;
		if (roll <= through6) return 6;
		return through6 + DWORD(uncalledChances[7]) < roll ? 0 : 7;
	}

	LONG GameCreature::CalculateUnarmedChance(AttackModeIterator attack, JE::Entity* victim,
		DWORD calledPart, GameWeapon* optionalWeapon)
	{
		if (!victim || attack == AttackModeIterator()) return 0;
		dynamic_cast<GameUsableObject*>(victim); //Original performs both RTTI queries.
		GameCreature* victimCreature=dynamic_cast<GameCreature*>(victim);
		DWORD accuracy=DWORD(GetTotalSkill(2))+attack->values18[2];
		if (optionalWeapon && optionalWeapon->currentAttackMode != AttackModeIterator())
		{
			DWORD kind;
			std::memcpy(&kind,attack->unwritten10,sizeof(kind));
			if ((kind==1 && optionalWeapon->weaponClass==9)
				|| (kind==3 && optionalWeapon->weaponClass==10))
				accuracy+=optionalWeapon->currentAttackMode->values18[2];
		}
		DWORD defense=victimCreature ? DWORD(victimCreature->GetDerivedAttribute(4)) : 0;
		float x=position.x-victim->position.x; //Original stores only the x difference as float.
		double y=double(position.y)-victim->position.y;
		double z=double(position.z)-victim->position.z;
		if (double(attack->range)<std::sqrt(z*z+y*y+double(x)*x)) return 0;
		if (calledPart!=9)
		{
			static const LONG penalties[8]={10,40,40,40,40,60,120,60};
			if (calledPart>=8)
				throw std::logic_error("GameCreature409F47: original called-part table outside recovered domain");
			defense+=DWORD(penalties[calledPart]);
		}
		LONG chance=LONG(accuracy-defense);
		if (chance<0) return 0;
		if (chance>95) return 95;
		return chance;
	}

	LONG GameCreature::CalculateUnarmedDamage(AttackModeIterator attack, JE::Entity* victim,
		DWORD, DWORD calledPart, GameWeapon* optionalWeapon)
	{
		if (attack == AttackModeIterator() && !victim) return 0; //Original tests AND, not OR.
		GameCreature* victimCreature=dynamic_cast<GameCreature*>(victim);
		GameUsableObject* usable=dynamic_cast<GameUsableObject*>(victim);
		if (calledPart==9 && !usable) return 0;
		DWORD high=attack->maximumDamage;
		if (attack->damageType==4) high+=DWORD(GetDerivedAttribute(6));
		DWORD result=attack->values18[1]+DWORD(CombatRandomRange(
			LONG(attack->minimumDamage),LONG(high)));
		if (optionalWeapon)
		{
			DWORD kind;
			std::memcpy(&kind,attack->unwritten10,sizeof(kind));
			if ((kind==1 && optionalWeapon->weaponClass==9)
				|| (kind==3 && optionalWeapon->weaponClass==10))
				throw std::logic_error("GameCreature40A045: original absolute ds:0x24 read");
		}
		LONG defense;
		if (usable) defense=LONG(usable->values164[attack->damageType]);
		else
		{
			switch (calledPart)
			{
			case 0: case 1: case 2: case 3: case 4: case 7:
				defense=victimCreature->resistances[attack->damageType]; break;
			case 5: case 6:
				defense=victimCreature->thresholds[attack->damageType]; break;
			default:
				JE::LogDebugString("GameCreature::CalculateUnarmedDamage() - Unknown CALLEDTARGET encountered.\n");
				defense=victimCreature->resistances[attack->damageType]; break;
			}
		}
		LONG half=defense/2;
		LONG damage=LONG(result-DWORD(half)-DWORD(CombatRandomRange(0,half)));
		return damage<0 ? 0 : damage;
	}
}
