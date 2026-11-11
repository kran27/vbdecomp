#pragma once

#include "EffectTreeTrigger.hpp"

namespace JE
{
	class EffectTreeCircleTrigger : public EffectTreeTrigger
	{
	public:
		EffectTreeCircleTrigger(); //5B22B0.
		virtual ~EffectTreeCircleTrigger(); //5B22D0; actual derived destruction phase.
		virtual BYTE Contains(const D3DXVECTOR3& point) const; //5B2350.
		virtual BYTE CreateShape(); //5B22E0.
		float radius; //+88, initial0.
	};

	class EffectTreeSemiCircleTrigger : public EffectTreeTrigger
	{
	public:
		EffectTreeSemiCircleTrigger(); //5B20D0.
		virtual ~EffectTreeSemiCircleTrigger(); //5B20F0; actual derived destruction phase.
		virtual BYTE Contains(const D3DXVECTOR3& point) const; //5B21B0.
		virtual BYTE CreateShape(); //5B2100.
		float radius; //+88, initial0.
		float angle; //+8C, constructor leaves unwritten.
	};

	class EffectTreeRingTrigger : public EffectTreeTrigger
	{
	public:
		EffectTreeRingTrigger(); //5B1FA0.
		virtual ~EffectTreeRingTrigger(); //5B1FD0; actual derived destruction phase.
		virtual BYTE Contains(const D3DXVECTOR3& point) const; //5B2050.
		virtual BYTE CreateShape(); //5B1FE0.
		float innerRadius, outerRadius; //+88/+8C, both initial0.
	};
}
