#pragma once

#include "entity/Action.hpp"

namespace F3
{
	class GameActionUseSkill : public JE::Action
	{
	public:
		GameActionUseSkill(); //407CA0.
		virtual BYTE IsComplete() const; //Shared406A50, completed+20.
		virtual void Cancel(); //Shared404B70.
		virtual int Start(DWORD time); //Shared404BB0.
		virtual int Update(DWORD time, DWORD budget); //407CC0.
		virtual int Finish(DWORD time); //Shared404BB0.
		DWORD targetID, skill; //Original+18/+1C; both initial0.
		BYTE completed; //Original+20, initial0.
	};
}
