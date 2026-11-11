#include <winsock2.h>
#include "GameEffectDamage.hpp"
#include "GameEntity.hpp"
#include "GameActionDie.hpp"
#include "CombatRandom.hpp"
#include "entity/World.hpp"
#include "entity/Gregorian.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/VisualEffect.hpp"
#include "graphics/VfxVeg.hpp"
#include "startup/GameServer.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cmath>
#include <stdexcept>

namespace F3
{
	GameEffectDamage::GameEffectDamage(DWORD effectType) : GameEffect(effectType)
	{
		amount = damageSubtype = damageType = 0;
		multiHit = 0;
		suppressReaction = 0;
		calledPart = 9;
	}
	GameEffectDamage::~GameEffectDamage() {}
	GameEffectDamage& GameEffectDamage::operator=(const GameEffectDamage& source)
	{
		GameEffect::operator=(source);
		amount=source.amount;
		return *this;
	}
	void GameEffectDamage::CopyFrom(const JE::Effect& source)
	{
		*this=static_cast<const GameEffectDamage&>(source);
	}
	BYTE GameEffectDamage::OnInitialize()
	{
		if (!BindEntities()) return 0;
		lifetimeMode=1;
		return 1;
	}
	void GameEffectDamage::OnDeactivate() {} //Actual shared404B70.
	BYTE GameEffectDamage::UnresolvedSlot34() { return 1; }
	BYTE GameEffectDamage::OnActivate()
	{
		completed=1; //45C6B3 precedes the calendar callback and all application effects.
		if (!GameEffect::OnActivate()) return 0;
		return ApplyDamage();
	}
	BYTE GameEffectDamage::ApplyDamage()
	{
		if (targetEntity->IsDead()) return 0;
		DWORD damage=amount;
		if (!damage) return 0; //Original integer comparison, not the decompiler's float test.
		targetEntity->SetHealth(LONG(DWORD(targetEntity->currentHealth)-damage));
		BYTE direction=0; //45C1B6 initializes the same physical stack byte used below.
		if (creatorID)
		{
			JE::Entity* source=JE::GetEntityPtr(creatorID);
			if (source)
			{
				D3DXVECTOR3 delta=source->position-targetEntity->position;
				D3DXVECTOR3 savedRotation=targetEntity->rotation;
				D3DXMATRIX inverseRotation;
				D3DXMatrixRotationYawPitchRoll(&inverseRotation,
					float(-double(savedRotation.x)*0.01745329238474369f),
					float(-double(savedRotation.y)*0.01745329238474369f),
					float(-double(savedRotation.z)*0.01745329238474369f));
				D3DXVec3TransformCoord(&delta,&delta,&inverseRotation);


				double angle=std::atan2(double(delta.z),double(delta.x));
				if ((angle<0.0 && angle>=-180.0) || (angle>=180.0 && angle<360.0))
				{
					direction=1;
				}
			}
		}
		if (!targetEntity->IsDead())
		{
			targetEntity->EndAnimationNow();
			if (!suppressReaction)
			{
				DWORD event=targetEntity->moving ? 33+(direction!=0)
					: multiHit ? (direction ? 41 : 38) : (direction ? 40 : 37);
				targetEntity->QueueAnimationEvent(event,0);
			}
			completed=1;
		}
		std::string effectName;
		switch (damageType)
		{
		case 0:
			effectName=damageSubtype==5 ? "FX_HitBloodHighVelocity_01.veg"
				: damage ? "FX_HitBloodStandard_01.veg" : "FX_HitRicochetMetal_01.veg";
			break;
		case 1: case 3: case 4: effectName="FX_HitBloodStandard_01.veg"; break;
		case 2: effectName="FX_HitElectrical_01.veg"; break;
		case 5:
			switch (damageSubtype)
			{
			case 1: effectName="FX_HitFire_01.veg"; break;
			case 2: effectName="FX_HitBloodStandard_01.veg"; break;
			case 3: effectName="FX_HitLaser_01.veg"; break;
			}
			break;
		}
		if (!effectName.empty())
		{
			JE::VisualEffect* effect=targetEntity->AttachVisualEffect(effectName.c_str(),0,1);
			DWORD socket=0;
			switch (calledPart)
			{
			case 0: socket=(CombatRandomRange(0,1)!=0)+1; break;
			case 1: socket=11-(CombatRandomRange(0,1)!=0); break;
			case 2: socket=6-(CombatRandomRange(0,1)!=0); break;
			case 3:
				socket=CombatRandomRange(0,1) ? 38 : (CombatRandomRange(0,1)!=0)+35; break;
			case 4:
				socket=CombatRandomRange(0,1) ? 23 : (CombatRandomRange(0,1)!=0)+20; break;
			case 5: case 6: socket=16; break;
			case 7: socket=CombatRandomRange(0,1) ? 9 : 4; break;
			}
			effect->instance->SetSocket(socket);
		}
		JE::Gregorian* calendar=JE::GetWorldServerInstance()->GetCalendar();
		const DWORD entityTime=targetEntity->value29C;
		cell arguments[4]={cell(calendar->GetTimeStamp()+entityTime),
			cell(creatorID),cell(damage),cell(damageType)};
		cell result; //Callback output only, original does not seed/read it otherwise.
		RunScript(targetEntity,"OnDamage",&result,4,arguments);
		static_cast<GameServer*>(g_pNetworkInstance->server)->SendEntityDamage(
			targetEntity->entityID,damage,damageType);
		if (targetEntity->IsDead())
		{
			targetEntity->ClearActions();
			JE::IWorld* world=JE::g_World;
			JE::Action* action=world->NewAction(0);
			GameActionDie* death=dynamic_cast<GameActionDie*>(action);
			death->damageType=damageType;
			death->direction=direction;
			death->damageSubtype=damageSubtype;
			targetEntity->QueueAction(action);
		}
		return 1;
	}
}
