#pragma once

#include "entity/ActionWait.hpp"

namespace F3
{
	class GameActionWait : public JE::ActionWait
	{
	public:
		GameActionWait(); //407ED0, no additional members.
		virtual ~GameActionWait(); //407EF0 ->shared4066F0, no owned children.
		virtual int Start(DWORD time); //407F00, ends current animation and queues default/idle.
	};
}
