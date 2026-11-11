#pragma once

#include "entity/ActionWander.hpp"

namespace F3
{
	class GameActionWander : public JE::ActionWander
	{
	public:
		GameActionWander() {} //46F140, actual base construction only.
		virtual float GetSpeed(); //407F80.
		virtual void GetWalkAnimation(DWORD& group, DWORD& file); //407F90.
		virtual void GetIdleAnimation(DWORD& group, DWORD& file); //Shared405790, original slot24.
	};
}
