#include "GameEntity.hpp"
#include "GameActionIdle.hpp"
#include "entity/World.hpp"
#include "model/ModelInstance.hpp"
#include "graphics/AnimationInstance.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "../startup/Random.hpp"
#include <stdlib.h>

namespace
{
	float RandomPercent() // Actual4B8310 with arguments0,100.
	{
		return static_cast<float>(rand() * 100.0 * 0.000030518509f);
	}
	DWORD RandomRange(LONG first, LONG last) // 48CD00, uses game's actual LCG48CCC0.
	{
		if (last == first) return first;
		if (last < first) { LONG swap = first; first = last; last = swap; }
		return first + F3::RandomNext() % (last - first + 1);
	}
}

namespace F3
{
	void GameEntity::OnActivate(DWORD time)
	{
		JE::Entity::OnActivate(time); // Original RunScript precedes state4 and effects.
		ApplyOnCreateEffects();
		InitializeActivationAnimation();
	}

	void GameEntity::InitializeActivationAnimation()
	{
		// Original467672..467773, factored from OnActivate for the explicit
		// workspace entry. It does not substitute the preceding script/state/effects.
		value2C = 5;
		QueueAnimationEvent(1, RandomPercent());
		if (model && model->animation)
		{
			LONG duration = LONG(static_cast<__int64>(double(GetAnimationDuration()) * 1000.0));
			DWORD phase = RandomRange(0, duration);
			animationStartTime = JE::g_World->GetTime() - phase;
			animationEndTime = duration + animationStartTime;
		}
		GameActionIdle* action = static_cast<GameActionIdle*>(JE::g_World->NewAction(1));
		action->percent = RandomPercent();
		QueueAction(action);
	}

	LONG GameEntity::QueueIdleVariant2(float percent)
	{
		if (value2C == 2 && weaponStance == 7)
			return QueueAnimationPercent(254, 0, 0, 0, percent, 0);
		return QueueIdle(percent);
	}

	LONG GameEntity::QueueIdleVariant3(float percent)
	{
		if (value2C == 2 && weaponStance == 7)
			return QueueAnimationPercent(255, 0, 0, 0, percent, 0);
		return QueueIdle(percent);
	}

	LONG GameEntity::QueueIdleVariant4(float percent)
	{
		if (value2C == 2 && weaponStance == 7)
			return QueueAnimationPercent(256, 0, 0, 0, percent, 0);
		return QueueIdle(percent);
	}

	LONG GameEntity::QueueIdleVariant5(float percent)
	{
		if (value2C == 2 && weaponStance == 7)
			return QueueAnimationPercent(257, 0, 0, 0, percent, 0);
		return QueueIdle(percent);
	}

	LONG GameEntity::QueueIdleVariant6(float percent)
	{
		if (value2C == 2 && weaponStance == 7)
			return QueueAnimationPercent(258, 0, 0, 0, percent, 0);
		return QueueIdle(percent);
	}

	LONG GameEntity::QueueAnimationEvent(DWORD event, float percent)
	{
		if (flag9C) return 0;
		if (event == 1) return QueueIdle(percent);
		if (event == 2) return QueueIdleVariant2(percent);
		if (event == 3) return QueueIdleVariant3(percent);
		if (event == 4) return QueueIdleVariant4(percent);
		if (event == 5) return QueueIdleVariant5(percent);
		if (event == 6) return QueueIdleVariant6(percent);
		if (event == 9) return QueueToDefault();
		if (event == 7) return QueueToCombat();
		if (event == 11) return QueueToCrouch();
		if (event == 29) return QueueLimp();
		if (event == 32) return QueueWalk();
		if (event == 21) return QueueEquip();
		if (event == 22) return QueueUnequip();
		if (event==17) return QueueProneBack(0);
		if (event==18) return QueueProneFront(0);
		if (event==19 || event==39) return QueueProneBack(1);
		if (event==20 || event==42) return QueueProneFront(1);
		if (event>=43 && event<=52)
		{
			flag9C=1; //4674C9, before the helper: later events stay blocked.
			return QueueDeathAnimation(event);
		}
		if (event >= 53 && event <= 104) return QueueAttackAnimation(event);
		if (event==33 || event==34 || event==37 || event==38 || event==40 || event==41)
			return QueueDamageReaction(event);
		if (event == 121) return QueueOperate();
		if (event == 120) return QueueReload();
		if (event == 141) return QueuePickup();
		JE::LogDebugString("GameEntity::QueueAnimEvent() - Unhandled ANIMEVENT.\n");
		return 0;
	}

	LONG GameEntity::QueueToDefault()
	{
		LONG result = 0;
		switch (value2C)
		{
		case 1: case 5: break;
		case 2:
			switch (weaponStance)
			{
			case 1: result = QueueAnimationPercent(56, 0, 1, 0, 0, 0); break;
			case 2: result = QueueAnimationPercent(87, 0, 1, 0, 0, 0); break;
			case 3: result = QueueAnimationPercent(119, 0, 1, 0, 0, 0); break;
			case 4: result = QueueAnimationPercent(150, 0, 1, 0, 0, 0); break;
			case 5: result = QueueAnimationPercent(186, 0, 1, 0, 0, 0); break;
			case 6: result = QueueAnimationPercent(217, 0, 1, 0, 0, 0); break;
			case 7: result = QueueAnimationPercent(292, 0, 1, 0, 0, 0); break;
			default:
				JE::LogDebugString("GameEntity::QueueToDefault() - Unknown weapon stance set.\n");
				break;
			}
			break;
		case 3:
			if (static_cast<LONG>(weaponStance) > 0 && static_cast<LONG>(weaponStance) <= 7)
			{
				result = QueueAnimationPercent(350, 0, 1, 0, 0, 0);
				result += QueueAnimationPercent(447, 0, 1, 0, 0, 0);
			}
			else JE::LogDebugString("GameEntity::QueueToDefault() - Unknown Weapon Stance Set.\n");
			break;
		case 4:
			if (static_cast<LONG>(weaponStance) > 0 && static_cast<LONG>(weaponStance) <= 7)
				result = QueueAnimationPercent(358, 0, 1, 0, 0, 0);
			//Original valid state4 also falls through this diagnostic.
			JE::LogDebugString("GameEntity::QueueToDefault() - Unknown Weapon Stance Set.\n");
			break;
		case 6:
			if (static_cast<LONG>(weaponStance) > 0 && static_cast<LONG>(weaponStance) <= 7)
				result = QueueAnimationPercent(447, 0, 1, 0, 0, 0);
			else JE::LogDebugString("GameEntity::QueueToDefault() - Unknown Weapon Stance Set.\n");
			break;
		case 7: case 8:
			result = QueueToCombat();
			value2C = 2;
			result += QueueToDefault();
			break;
		case 9:
			result = QueueAnimationPercent(513, 0, 1, 0, 0, 0);
			break;
		default:
			JE::LogDebugString("GameEntity::QueueToDefault() - Unknown RootAnimState.\n");
		}
		value2C = 5;
		return result;
	}


}
