#pragma once

#include "Trigger.hpp"
#include <list>
#include <map>

class OCTREE_Shape;

namespace JE
{
	class Effect;
	// Gameplay effect area, distinct from graphics VFX and Gfx_Light owners.
	class EffectTreeTrigger : public Trigger
	{
	public:
		EffectTreeTrigger(); //5AAD50.
		virtual ~EffectTreeTrigger(); //5AAB40.
		virtual BYTE Read(Resources::ResourceMemoryStream* stream); //409BB0, no reads.
		virtual void OnEnter(Entity* entity); //5AAE40.
		virtual void OnLeave(Entity* entity); //5AA630.
		virtual void OnUpdate(DWORD time); //5AA750.
		virtual void ReevaluateEntities(); //5AA7E0, original+24.
		virtual BYTE CreateShape(); //5AA530, original+28.
		void SetPosition(const D3DXVECTOR3& value); //5AA4E0.

		D3DXVECTOR3 position; //+30, ctor zero.
		Common::Polygon localPolygon; //+3C, same native owner as Trigger+4.
		DWORD creatorID, attachmentOwnerID; //+50/+54, ctor0.
		Entity* attachedEntity; //+58, ctorNULL; deferred Destroy on trigger deletion.
		std::list<Effect*> templates; //+5C, owned effect trees.
		std::map<DWORD,std::list<DWORD> > appliedEffects; //+68, Entity ID -> effect IDs.
		BYTE reevaluate; //+74, ctor1.
		OCTREE_Shape* shape; //+78, owned via Release.
		BYTE forceShapeMembership; //+7C, ctor0; derived shapes set1.
		DWORD expirationTime; //+80, ctor0; unsigned absolute clock.
		DWORD targetLimit; //+84, ctorFFFFFFFF; exact equality gate.
	private:
		EffectTreeTrigger(const EffectTreeTrigger&);
		EffectTreeTrigger& operator=(const EffectTreeTrigger&);
	};
}
