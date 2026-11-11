#pragma once

#include "Action.hpp"
#include <d3dx8.h>

namespace JE
{
	struct PathRequest;
	class ActionMoveTo : public Action
	{
	public:
		ActionMoveTo(); //55B300.
		virtual ~ActionMoveTo(); //55B460; pending request is unqueued before destruction.
		virtual BYTE IsComplete() const; //55B440.
		virtual void Cancel(); //55B450.
		virtual int Start(DWORD time); //55B4F0.
		virtual int Update(DWORD time, DWORD budget); //55B600.
		virtual int Finish(DWORD time); //55B3F0.
		virtual float GetSpeed(); //55B340.
		virtual void GetWalkAnimation(DWORD& group, DWORD& file); //Shared406E90.
		virtual void GetIdleAnimation(DWORD& group, DWORD& file); //Shared406E90.
		virtual void StartMoving(); //55B370.
		virtual void StopMoving(); //55B3B0.
		virtual void OnMoved(); //Shared404B70; game4060B0 handles occupied footprints.
		void SetTarget(Entity* entity); //55B350; NULL retains the prior targetID.
		void SetCompleted(BYTE value); //55B420; also publishes Entity moving byte.
		PathRequest* request; //+18, owned.
		Entity* target; //+1C, borrowed cached lookup.
		DWORD targetID; //+20.
		D3DXVECTOR3 destination; //+24.
		DWORD stopDistance; //+30, default5.
		BYTE flag34, movingStarted; //+34/+35.
		DWORD reachability; //+38.
		BYTE checkReachability, completed; //+3C/+3D.
		DWORD lastStep, lastRepath; //+40/+44; unwritten until movement starts.
	private:
		ActionMoveTo(const ActionMoveTo&);
		ActionMoveTo& operator=(const ActionMoveTo&);
	};
}
