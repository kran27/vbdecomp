#include <winsock2.h>
#include "GameEffect.hpp"
#include "GameEntity.hpp"
#include "GameCreature.hpp"
#include "GameUsableObject.hpp"
#include "CombatRandom.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/World.hpp"
#include "entity/Gregorian.hpp"
#include "entity/EntityEnvironment.hpp"
#include "model/CommonException.hpp"
#include <stdexcept>

namespace F3
{
	GameEffect::GameEffect(DWORD effectType) : Effect(effectType)
	{
		targetEntity = NULL;
		targetCreature = NULL;
		sourceEntity = NULL;
		sourceCreature = NULL;
		value5C = 0;
		minimumRoll = maximumRoll = 0;
		lifetimeMode = 0;
		minimumDuration = maximumDuration = 0;
		durationUnit = 0;
		fixedRoll = fixedDuration = 0;
		value88 = 0;
		startTimestamp = duration = 0;
		modifier = 0;
		value84 = 1;
		//Original leaves value58, value80 and value8C unwritten.
	}
	GameEffect::~GameEffect() {}
	DWORD GameEffect::GetRole() const { return value58; }
	GameEffect& GameEffect::operator=(const GameEffect& source)
	{
		Effect::operator=(source);
		targetEntity=source.targetEntity; targetCreature=source.targetCreature;
		sourceEntity=source.sourceEntity; sourceCreature=source.sourceCreature;
		value58=source.value58; value5C=source.value5C;
		minimumRoll=source.minimumRoll; maximumRoll=source.maximumRoll;
		lifetimeMode=source.lifetimeMode; minimumDuration=source.minimumDuration;
		maximumDuration=source.maximumDuration; durationUnit=source.durationUnit;
		fixedRoll=source.fixedRoll; fixedDuration=source.fixedDuration;
		value80=source.value80; value84=source.value84; value88=source.value88; value8C=source.value8C;
		startTimestamp=source.startTimestamp; duration=source.duration; modifier=source.modifier;
		return *this;
	}
	void GameEffect::CopyFrom(const JE::Effect& source)
	{
		*this=static_cast<const GameEffect&>(source);
	}
	BYTE GameEffect::Write(FILE* stream)
	{
		return Effect::Write(stream)!=0;
	}
	BYTE GameEffect::ReadFromResource(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream)
	{
		return Effect::ReadFromResource(chunk,stream)!=0;
	}
	BYTE GameEffect::Read(FILE* stream)
	{
		return Effect::Read(stream)!=0;
	}
	void GameEffect::OnBegin() {}
	BYTE GameEffect::OnActivate()
	{
		if (!(g_pNetworkInstance->GetMode()&1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		JE::IWorldServer* server=JE::g_World;
		JE::Gregorian* calendar=server->GetCalendar();
		const DWORD entityTime=targetEntity->value29C;
		startTimestamp=entityTime+calendar->GetTimeStamp();
		return 1;
	}
	BYTE GameEffect::OnUpdate()
	{
		if (!activated) ActivateTree();
		if (lifetimeMode==1)
		{
			completed=1;
			return 0;
		}
		if (lifetimeMode) return 1;
		JE::Gregorian* calendar=JE::GetWorldServerInstance()->GetCalendar();
		const DWORD entityTime=targetEntity->value29C;
		const DWORD currentTime=calendar->GetTimeStamp()+entityTime;
		if (currentTime-startTimestamp>=duration)
		{
			completed=1;
			return 0;
		}
		return 1;
	}
	LONG GameEffect::GetRoll()
	{
		LONG roll=fixedRoll ? fixedRoll : CombatRandomRange(minimumRoll,maximumRoll);
		return LONG(DWORD(roll)+DWORD(modifier));
	}
	DWORD GameEffect::GetDuration()
	{
		if (lifetimeMode) return 0;
		if (fixedDuration) return fixedDuration;
		DWORD value=DWORD(CombatRandomRange(LONG(minimumDuration),LONG(maximumDuration)));
		if (!(g_pNetworkInstance->GetMode()&1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		JE::IWorldServer* server=JE::g_World;
		JE::Gregorian* calendar=server->GetCalendar();
		switch (durationUnit)
		{
		case 0:
			value*=JE::GetWorldServerInstance()->GetSecondsPerRound();
			return value;
		case 2:
		{
			DWORD seconds=calendar->SecondsPerMinute();
			seconds*=calendar->MinutesPerHour();
			value*=seconds;
			return value;
		}
		case 3:
		{
			DWORD seconds=calendar->SecondsPerMinute();
			seconds*=calendar->MinutesPerHour();
			seconds*=calendar->HoursPerDay();
			value*=seconds;
			return value;
		}
		case 4:
			value*=calendar->SecondsPerMinute();
			return value;
		default:
			return value;
		}
	}
	BYTE GameEffect::BindEntities()
	{
		JE::Entity* victim=JE::GetEntityPtr(ownerID);
		if (!victim) return 0;
		targetEntity=dynamic_cast<GameEntity*>(victim);
		if (!targetEntity) return 0;
		targetCreature=dynamic_cast<GameCreature*>(targetEntity);
		GameUsableObject* usable=dynamic_cast<GameUsableObject*>(targetEntity);
		if (!targetCreature && !usable) return 0;
		JE::Entity* source=JE::GetEntityPtr(creatorID);
		if (!source) return 0;
		sourceEntity=dynamic_cast<GameEntity*>(source);
		if (!sourceEntity) return 0;
		sourceCreature=dynamic_cast<GameCreature*>(sourceEntity);
		return sourceCreature!=NULL;
	}
}
