#pragma once

#include "entity/ActionPickUp.hpp"

namespace F3
{
	class GameActionPickUp : public JE::ActionPickUp
	{
	public:
		GameActionPickUp(); //406E70; actual base construction, no additional fields.
		virtual ~GameActionPickUp(); //4071A0 ->406EB0/shared4066F0.
		virtual int Update(DWORD time,DWORD budget); //406F10, cost12 only when animation starts.
		LONG StartPickupAnimation(); //406EC0; EndAnimationNow then event141.
		void OnPickupCue(); //407170; actual PU: server remove from map then inventory add.
	};
}
