#pragma once

#include "entity/Action.hpp"

namespace F3
{
	class GameActionOperateContainer : public JE::Action
	{
	public:
		GameActionOperateContainer(); //406A10; kind/minimumBudget remain unwritten.
		virtual BYTE IsComplete() const; //406A50.
		virtual void Cancel(); //Shared404B70.
		virtual int Start(DWORD time); //Shared404BB0.
		virtual int Update(DWORD time,DWORD budget); //406A60; no budget predicate, returns12 on inspect path.
		virtual int Finish(DWORD time); //Shared404BB0.
		void SetTarget(JE::Entity* entity); //Shared406A30, NULL retains previous ID.
		JE::Entity* target; //Original+18, borrowed/refreshed each Update.
		DWORD targetID; //Original+1C.
		BYTE completed; //Original+20, ctor0.
	};
}
