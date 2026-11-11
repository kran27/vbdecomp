#pragma once
#include "GameEffect.hpp"

namespace F3
{
	class GameEffectDamage : public GameEffect
	{
	public:
		explicit GameEffectDamage(DWORD type); //45C040.
		virtual ~GameEffectDamage(); //45C0B0.
		GameEffectDamage& operator=(const GameEffectDamage& source); //45C0C0; only amount follows the base copy.
		virtual void CopyFrom(const JE::Effect& source); //45C0F0 forwarding entry, original slot8.
		virtual BYTE OnInitialize(); //45C100, bind then one-shot lifetime.
		virtual BYTE OnActivate(); //45C6B0, completes before applying the damage.
		virtual void OnDeactivate(); //404B70.
		virtual BYTE UnresolvedSlot34(); //Original+34, shared408D30 returns AL1, RET0.
		DWORD amount, damageType, damageSubtype, calledPart;
		BYTE multiHit, suppressReaction;
	private:
		BYTE ApplyDamage(); //45C140 shared body.
	};
}
