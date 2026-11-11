#pragma once

#include "Action.hpp"

namespace JE
{
	class ActionDie : public Action
	{
	public:
		ActionDie(); //55FB10; owner/minimumBudget remain unwritten.
		virtual ~ActionDie() {}
		virtual BYTE IsComplete() const; //Shared404B60.
		virtual void Cancel(); //Shared404B70, no state mutation.
		virtual int Start(DWORD time); //55FB30 ->Entity.Die551CA0.
		virtual int Update(DWORD time, DWORD budget); //5638D0, unchanged0 hook.
		virtual int Finish(DWORD time); //Shared404BB0.
		BYTE completed; //Original+18, ctor0; base Update never changes it.
	};
}
