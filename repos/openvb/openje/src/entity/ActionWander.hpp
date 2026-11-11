#pragma once

#include "Action.hpp"
#include <d3dx8math.h>

namespace JE
{
	class ActionWander : public Action
	{
	public:
		ActionWander(); //5A27E0.
		virtual BYTE IsComplete() const; //Shared408D30, always1.
		virtual void Cancel(); //Shared404B70.
		virtual int Start(DWORD time); //5A28E0.
		virtual int Update(DWORD time, DWORD budget); //5A28F0.
		virtual int Finish(DWORD time); //Shared404BB0.
		void SetBounds(const D3DXVECTOR3& first, const D3DXVECTOR3& last); //5A2820.
		void ChooseDestination(); //5A2850.
		D3DXVECTOR3 upperLeft, lowerRight; //Original+18/+24.
		DWORD maximumWait, repetitions; //Original+30/+34, defaults6000/0.
		D3DXVECTOR3 destination; //Original+38; shared Action layout remains open.
	};
}
