#pragma once

#include "entity/Action.hpp"

namespace F3
{
	class GameActionInitiateDialogue : public JE::Action
	{
	public:
		GameActionInitiateDialogue(); //404B40.
		virtual BYTE IsComplete() const; //Shared404B60.
		virtual void Cancel(); //Shared404B70.
		virtual int Start(DWORD time); //404B80.
		virtual int Update(DWORD time, DWORD budget); //404BE0.
		virtual int Finish(DWORD time); //Shared404BB0.
		BYTE completed, useTargetDialogue;
		JE::Entity* target;
		DWORD targetID;
	};
}
