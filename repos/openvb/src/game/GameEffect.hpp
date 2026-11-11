#pragma once
#include "entity/Effect.hpp"

namespace F3
{
	class GameEntity;
	class GameCreature;
	class GameEffect : public JE::Effect
	{
	public:
		explicit GameEffect(DWORD type); //462470.
		virtual ~GameEffect(); //4624E0.
		virtual DWORD GetRole() const; //45C0A0; value58 is constructor-unwritten.
		GameEffect& operator=(const GameEffect& source); //4624F0; base first, then all game fields.
		virtual void CopyFrom(const JE::Effect& source); //4625B0 forwarding entry, original slot8.
		virtual BYTE Write(FILE* stream); //4625C0, qualified base then normalize AL.
		virtual BYTE ReadFromResource(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream); //462600.
		virtual BYTE Read(FILE* stream); //4625E0, qualified base then normalize AL.
		virtual void OnBegin(); //404B70, actual empty callback.
		virtual BYTE OnActivate(); //462620, capture current calendar timestamp.
		virtual BYTE OnUpdate(); //4626A0, elapsed-time/one-shot completion.
		virtual LONG GetRoll(); //462710, original slot38.
		virtual DWORD GetDuration(); //462740, slot3C; original round/second/hour/day/minute conversion.
		BYTE BindEntities(); //462860.
		GameEntity* targetEntity;
		GameCreature* targetCreature;
		GameEntity* sourceEntity;
		GameCreature* sourceCreature;
		DWORD value58, value5C;
		LONG minimumRoll, maximumRoll;
		DWORD lifetimeMode, minimumDuration, maximumDuration, durationUnit;
		LONG fixedRoll;
		DWORD fixedDuration, value80, value84, value88, value8C;
		DWORD startTimestamp, duration;
		LONG modifier;
	};
}
