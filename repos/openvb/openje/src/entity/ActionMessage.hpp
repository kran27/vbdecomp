#pragma once

#include "Action.hpp"
extern "C"
{
#include "amx.h"
}

namespace JE
{
	class ActionMessage : public Action
	{
	public:
		ActionMessage(); //5A8FF0, actual RTTI ActionMessage.
		virtual BYTE IsComplete() const; //404B60.
		virtual void Cancel(); //404B70, no argument cleanup.
		virtual int Start(DWORD time); //5A9080, sends and releases the copied cells.
		virtual int Update(DWORD time, DWORD budget); //5A9070.
		virtual int Finish(DWORD time); //404BB0.
		void SetArguments(LONG count, const cell* values); //5A9020.
		BYTE completed;
		DWORD targetID, teamID, squadID, message;
		LONG argumentCount;
		cell* arguments;
		//The original destructor has no owned-child cleanup. Start frees arguments
		//without clearing the pointer; cancellation before Start leaks that allocation.
	};
}
