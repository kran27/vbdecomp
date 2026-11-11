#pragma once

#include "Action.hpp"

namespace JE
{
	class ActionPickUp : public Action
	{
	public:
		ActionPickUp(); //563520; target/deadlines0, firstUpdate1, completed0.
		virtual ~ActionPickUp(); //405350 ->shared4066F0, no owned payload.
		virtual BYTE IsComplete() const; //407200.
		virtual void Cancel(); //Shared404B70.
		virtual int Start(DWORD time); //Shared404BB0.
		virtual int Update(DWORD time,DWORD budget); //563550.
		virtual int Finish(DWORD time); //Shared404BB0.
		virtual void GetApproachAnimation(DWORD& group,DWORD& file); //Original+1C, shared406E90.
		virtual void GetPickupAnimation(DWORD& group,DWORD& file); //Original+20, shared406E90.
		virtual void GetRecoveryAnimation(DWORD& group,DWORD& file); //Original+24, shared406E90.
		virtual void GetIdleAnimation(DWORD& group,DWORD& file); //Original+28, shared406E90.
		void SetTarget(Entity* entity); //406A30; NULL does not clear the retained ID.
		Entity* target; //Original+18; borrowed, refreshed from targetID every update.
		DWORD targetID; //Original+1C.
		DWORD approachEnd,pickupEnd,recoveryEnd; //Original+20/+24/+28; zero sentinel.
		BYTE firstUpdate,completed; //Original+2C/+2D.
	protected:
		void FaceTarget(); //Shared source factor of563681/407082; preserves vertical normalization.
	};
}
