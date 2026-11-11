#include "GameEntity.hpp"
#include "model/ModelInstance.hpp"
#include "graphics/AnimationInstance.hpp"
#include "graphics/AnimationObject.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <stdlib.h>
#include <stdexcept>

namespace
{
	// Original516-DWORD proprietary fallback data6F3960..6F4170. The following
	// original table6F4170 holds the516 animation-group resource suffix strings.
	const DWORD animationFallbacks[516] =
	{
		0, 0, 1, 0, 0, 0, 0, 231, // 0..7
		7, 8, 9, 43, 232, 15, 15, 43, // 8..15
		13, 19, 17, 245, 19, 65, 247, 26, // 16..23
		64, 0, 253, 269, 270, 273, 62, 63, // 24..31
		26, 43, 36, 36, 43, 34, 50, 38, // 32..39
		286, 62, 63, 0, 46, 46, 43, 44, // 40..47
		50, 48, 0, 290, 11, 0, 50, 291, // 48..55
		292, 293, 294, 295, 26, 299, 300, 301, // 56..63
		0, 271, 272, 12, 70, 68, 19, 70, // 64..71
		22, 23, 26, 27, 28, 29, 79, 31, // 72..79
		32, 40, 0, 0, 50, 51, 84, 0, // 80..87
		57, 58, 59, 60, 61, 62, 63, 65, // 88..95
		66, 116, 12, 101, 99, 19, 101, 22, // 96..103
		23, 26, 27, 28, 29, 30, 31, 32, // 104..111
		116, 40, 41, 42, 43, 51, 97, 56, // 112..119
		57, 58, 59, 60, 61, 62, 63, 65, // 120..127
		66, 12, 132, 130, 19, 132, 22, 23, // 128..135
		26, 27, 28, 29, 30, 31, 32, 40, // 136..143
		41, 42, 50, 51, 53, 146, 56, 57, // 144..151
		58, 59, 60, 61, 62, 63, 65, 66, // 152..159
		183, 12, 164, 162, 19, 164, 22, 23, // 160..167
		11, 33, 43, 52, 26, 27, 28, 29, // 168..175
		30, 31, 32, 183, 40, 41, 42, 43, // 176..183
		51, 160, 56, 57, 58, 59, 60, 61, // 184..191
		62, 63, 64, 65, 66, 98, 200, 198, // 192..199
		101, 200, 103, 104, 105, 106, 107, 108, // 200..207
		109, 110, 111, 215, 113, 114, 115, 116, // 208..215
		117, 119, 120, 121, 122, 123, 124, 125, // 216..223
		126, 127, 128, 0, 0, 0, 229, 230, // 224..231
		0, 271, 271, 271, 271, 271, 271, 271, // 232..239
		271, 271, 278, 245, 243, 0, 245, 0, // 240..247
		249, 0, 0, 0, 0, 0, 253, 253, // 248..255
		253, 253, 253, 262, 262, 262, 0, 262, // 256..263
		262, 262, 262, 262, 262, 0, 269, 0, // 264..271
		271, 299, 300, 301, 278, 278, 0, 278, // 272..279
		278, 278, 278, 278, 278, 278, 299, 300, // 280..287
		301, 0, 0, 0, 0, 0, 0, 0, // 288..295
		0, 0, 0, 0, 243, 245, 0, 0, // 296..303
		0, 333, 305, 323, 335, 311, 309, 338, // 304..311
		311, 0, 0, 342, 309, 311, 343, 344, // 312..319
		345, 0, 323, 0, 0, 307, 0, 349, // 320..327
		350, 351, 0, 0, 0, 0, 333, 0, // 328..335
		338, 336, 0, 338, 0, 0, 0, 0, // 336..343
		0, 0, 0, 346, 0, 0, 0, 0, // 344..351
		0, 0, 353, 0, 0, 0, 0, 0, // 352..359
		401, 26, 29, 40, 61, 423, 425, 57, // 360..367
		58, 59, 401, 74, 423, 88, 89, 90, // 368..375
		401, 105, 423, 120, 121, 122, 401, 136, // 376..383
		423, 151, 152, 153, 401, 172, 423, 187, // 384..391
		188, 189, 401, 204, 423, 218, 219, 220, // 392..399
		0, 0, 0, 0, 0, 0, 0, 0, // 400..407
		0, 0, 0, 0, 0, 0, 0, 0, // 408..415
		253, 286, 0, 0, 0, 0, 0, 0, // 416..423
		0, 0, 0, 0, 0, 0, 0, 0, // 424..431
		0, 0, 0, 302, 443, 315, 318, 446, // 432..439
		447, 329, 449, 0, 342, 343, 0, 0, // 440..447
		351, 0, 0, 0, 0, 0, 0, 0, // 448..455
		0, 0, 0, 0, 0, 0, 0, 0, // 456..463
		0, 0, 0, 0, 0, 0, 0, 0, // 464..471
		0, 0, 0, 488, 489, 488, 488, 488, // 472..479
		488, 488, 488, 488, 0, 0, 485, 0, // 480..487
		0, 0, 0, 503, 503, 503, 503, 503, // 488..495
		503, 503, 503, 0, 0, 500, 0, 0, // 496..503
		0, 0, 505, 0, 0, 0, 0, 0, // 504..511
		0, 0, 0, 0 // 512..515
	};

	DWORD BadWeaponStance(DWORD fallback)
	{
		JE::LogDebugString("GameEntity::QueryDefaultAnimation() - Unknown Weapon Stance set.\n");
		return fallback;
	}
}

namespace F3
{
	void GameEntity::ResolveAnimation(DWORD group, DWORD file, DWORD* outputGroup,
		DWORD* outputFile)
	{
		// Original leaves caller output storage untouched when either owner is absent.
		if (!model || !model->animation) return;
		Gfx_AnimationInstance* animation = model->animation;
		*outputGroup = group;
		*outputFile = file;
		if (group == DWORD(-1) || file == DWORD(-1)) return;
		while (*outputGroup)
		{
			DWORD current = *outputGroup;
			if (animation->animationObject->HasAnimation(current, *outputFile, 0)) break;
			// The original table has no index guard. Unsupported external IDs remain
			// explicit rather than reading a neighboring original global in the host.
			if (current >= 516)
				throw std::logic_error("GameEntity467830: animation group outside original table.");
			*outputGroup = animationFallbacks[current];
			*outputFile = 0;
		}
	}

	BYTE GameEntity::IsDead() const
	{
		return JE::Entity::IsDead() || currentHealth <= 0;
	}

	DWORD GameEntity::GetIdleAnimationGroup() const
	{
		LONG stance = static_cast<LONG>(weaponStance);
		switch (value2C)
		{
		case 1:
			return 0;
		case 2:
			switch (stance)
			{
			case 1: return 26;
			case 2: return 74;
			case 3: return 105;
			case 4: return 136;
			case 5: return 172;
			case 6: return 204;
			case 7: return 253;
			default: return BadWeaponStance(253);
			}
		case 3:
			if (stance >= 1 && stance <= 6) return 318;
			if (stance == 7) return 343;
			return BadWeaponStance(343);
		case 4:
			if (stance >= 1 && stance <= 7) return 355;
			return BadWeaponStance(355);
		case 5:
			switch (stance)
			{
			case 1: return 361;
			case 2: return 371;
			case 3: return 377;
			case 4: return 383;
			case 5: return 389;
			case 6: return 395;
			case 7: return 416;
			default: return BadWeaponStance(416);
			}
		case 6:
			if (stance >= 1 && stance <= 6) return 438;
			if (stance == 7) return 445;
			return BadWeaponStance(445);
		case 7:
			if (stance >= 1 && stance <= 7) return 487;
			return BadWeaponStance(487);
		case 8:
			if (stance >= 1 && stance <= 7) return 502;
			return BadWeaponStance(502);
		case 9:
			if (stance >= 1 && stance <= 7) return 511;
			return BadWeaponStance(511);
		default:
			JE::LogDebugString("GameEntity::QueryDefaultAnimation() - Unknown Animation State set.\n");
			return 0;
		}
	}

	LONG GameEntity::QueueIdle(float percent)
	{
		LONG result = 0;
		switch (value2C)
		{
		case 1: break;
		case 2:
			switch (weaponStance)
			{
			case 1: result = QueueAnimationPercent(26,0,0,0,percent,0); break;
			case 2: result = QueueAnimationPercent(74,0,0,0,percent,0); break;
			case 3: result = QueueAnimationPercent(105,0,0,0,percent,0); break;
			case 4: result = QueueAnimationPercent(136,0,0,0,percent,0); break;
			case 5: result = QueueAnimationPercent(172,0,0,0,percent,0); break;
			case 6: result = QueueAnimationPercent(204,0,0,0,percent,0); break;
			case 7: result = QueueAnimationPercent(253,0,0,0,percent,0); break;
			default: JE::LogDebugString("GameEntity::Queue_Idle() - Unknown Weapon Stance set.\n");
			}
			break;
		case 3:
			switch (static_cast<LONG>(weaponStance))
			{
			case 7: result = QueueAnimationPercent(343,0,0,0,percent,0); break;
			case 1: case 2: case 3: case 4: case 5: case 6:
				result = QueueAnimationPercent(318,0,0,0,percent,0); break;
			default: JE::LogDebugString("GameEntity::Queue_Idle() - Unknown Weapon Stance set.\n");
			}
			break;
		case 4:
			if (static_cast<LONG>(weaponStance) > 0 && static_cast<LONG>(weaponStance) <= 7)
				result = QueueAnimationPercent(355,0,0,0,percent,0);
			else JE::LogDebugString("GameEntity::Queue_Idle() - Unknown Weapon Stance set.\n");
			break;
		case 5:
			switch (weaponStance)
			{
			case 1: result = QueueAnimationPercent(361,0,0,0,percent,0); break;
			case 2: result = QueueAnimationPercent(371,0,0,0,percent,0); break;
			case 3: result = QueueAnimationPercent(377,0,0,0,percent,0); break;
			case 4: result = QueueAnimationPercent(383,0,0,0,percent,0); break;
			case 5: result = QueueAnimationPercent(389,0,0,0,percent,0); break;
			case 6: result = QueueAnimationPercent(395,0,0,0,percent,0); break;
			case 7: result = QueueAnimationPercent(416,0,0,0,percent,0); break;
			default: JE::LogDebugString("GameEntity::Queue_Idle() - Unknown Weapon Stance set.\n");
			}
			break;
		case 6:
			if (static_cast<LONG>(weaponStance) > 0 && static_cast<LONG>(weaponStance) <= 7)
				result = QueueAnimationPercent(445,0,0,0,percent,0);
			else JE::LogDebugString("GameEntity::Queue_Idle() - Unknown Weapon Stance set.\n");
			break;
		case 7:
			if (static_cast<LONG>(weaponStance) > 0 && static_cast<LONG>(weaponStance) <= 7)
				result = QueueAnimationPercent(487,0,0,0,percent,0);
			else JE::LogDebugString("GameEntity::Queue_Idle() - Unknown Weapon Stance set.\n");
			break;
		case 8:
			if (static_cast<LONG>(weaponStance) > 0 && static_cast<LONG>(weaponStance) <= 7)
				result = QueueAnimationPercent(502,0,0,0,percent,0);
			else JE::LogDebugString("GameEntity::Queue_Idle() - Unknown Weapon Stance set.\n");
			break;
		case 9:
			if (static_cast<LONG>(weaponStance) > 0 && static_cast<LONG>(weaponStance) <= 7)
				result = QueueAnimationPercent(511,0,0,0,percent,0);
			else JE::LogDebugString("GameEntity::Queue_Idle() - Unknown Weapon Stance set.\n");
			break;
		default:
			JE::LogDebugString("GameEntity::Queue_Idle() - Unknown Animation State set.\n");
		}
		return result;
	}

	void GameEntity::InitializeIdleAnimationState()
	{
		value2C = 5; // Actual activation467672; this is not an arbitrary idle selection.
		float percent = static_cast<float>(rand() * 100.0 * 0.000030518509f);
		// Actual event1 dispatcher4673D0 checks flag9C after the random argument
		// has already been evaluated by467640.
		if (!flag9C) QueueIdle(percent);
	}
}
