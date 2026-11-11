#include "game/ActionbarInterface.hpp"

namespace F3
{
	void ActionbarInterface::SetCompanionMode(DWORD mode) //4172D0.
	{
		value0C = mode;
		RefreshCompanions();
	}

	void ActionbarInterface::CycleCompanionMode() //4172E0.
	{
		if (value0C == 1) value0C = 2;
		else if (value0C == 2) value0C = 1;
		else return;
		RefreshCompanions();
	}
}
