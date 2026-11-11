#pragma once
#include "Action.hpp"

namespace JE
{
	DWORD FloatingSpeechDuration(DWORD length); //Shared original expression55FC55/5C27FE.
	class ActionFloatingSpeechText : public Action
	{
	public:
		ActionFloatingSpeechText(); //55FB60; kind/owner/budget/text/duration/deadline unwritten.
		void SetCompleted(BYTE value); //55FB90; raw completion byte, ordinary RET4.
		virtual BYTE IsComplete() const; //55FBA0.
		virtual void Cancel(); //Shared404B70.
		virtual int Start(DWORD time); //Shared404BB0.
		virtual int Update(DWORD time,DWORD budget); //55FBB0.
		virtual int Finish(DWORD time); //Shared404BB0.
		DWORD stringReference,duration,red,green,blue; //Original+18..28.
		BYTE completed,sent; //+2C/+2D, initial0.
		DWORD deadline; //+30, written after the first broadcast/text lookup.
	};
}
