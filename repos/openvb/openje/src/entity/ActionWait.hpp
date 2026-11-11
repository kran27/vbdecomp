#pragma once

#include "Action.hpp"

namespace JE
{
	class ActionWait : public Action
	{
	public:
		ActionWait(); //5601A0; owner/minimumBudget remain unwritten.
		virtual ~ActionWait(); //Separate base-destruction boundary, shared4066F0.
		virtual BYTE IsComplete() const; //Shared404B60.
		virtual void Cancel(); //Shared404B70, no state change.
		virtual int Start(DWORD time); //5601C0.
		virtual int Update(DWORD time, DWORD budget); //5601D0, strict unsigned > deadline.
		virtual int Finish(DWORD time); //Shared404BB0.
		DWORD SetDuration(DWORD milliseconds); //568420, consumed assignment result.
		BYTE completed; //Original+18, initially0.
		DWORD startTime, duration; //+1C/+20, initially0.
	};
}
