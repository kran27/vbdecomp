#pragma once

#include "entity/Action.hpp"

namespace F3
{
	class GameActionIdle : public JE::ActionIdle
	{
	public:
		GameActionIdle(); // 4055F0, including ActionIdle560180.
		virtual int Start(DWORD time); // 405620.
		float percent;
		BYTE endCurrent; // Original+20 is a byte; following padding stays unwritten.
	};
}
