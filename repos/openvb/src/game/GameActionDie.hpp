#pragma once

#include "entity/ActionDie.hpp"

namespace F3
{
	class GameActionDie : public JE::ActionDie
	{
	public:
		GameActionDie(); //404E50.
		virtual ~GameActionDie() {}
		virtual int Start(DWORD time); //404E90.
		virtual int Update(DWORD time, DWORD budget); //405020.
		DWORD damageType, damageSubtype, animationEnd;
		BYTE direction;
	};
}
