#pragma once

#include "entity/Action.hpp"

namespace F3
{
	class GameDoor;
	class GameActionOperateDoor : public JE::Action
	{
	public:
		GameActionOperateDoor(); //4066C0, actual RTTI name.
		virtual BYTE IsComplete() const; //406720.
		virtual void Cancel(); //Shared404B70.
		virtual int Start(DWORD time); //Shared404BB0.
		virtual int Update(DWORD time, DWORD budget); //4067C0.
		virtual int Finish(DWORD time); //Shared404BB0.
		void SetDoor(GameDoor* value); //406700, NULL retains prior ID.
		GameDoor* door; //+18, borrowed cached lookup.
		DWORD doorID, deadline; //+1C/+20.
		BYTE firstUpdate, completed; //+24/+25.
	private:
		DWORD StartOperating(); //406730.
		void StopOperating(); //406780.
	};
}
