#pragma once

#include "entity/Action.hpp"

namespace JE { namespace Net { class Packet; } }

namespace F3
{
	class GameCreature;

	// Original30h owner. The shared Action base is still a source adaptation;
	// its original18h layout has not yet been restored. Do not infer a matching
	// class layout from the original member offsets recorded below.
	class GameActionGive : public JE::Action
	{
	public:
		GameActionGive(); //4052A0; kind/starting1, payload0, minimumBudget unwritten.
		virtual BYTE IsComplete() const; //Shared404B60, original completed+18.
		virtual void Cancel(); //Shared404B70.
		virtual int Start(DWORD time); //405300; actual GameCreature giver required.
		virtual int Update(DWORD time, DWORD budget); //405370, includes29B EH.
		virtual int Finish(DWORD time); //Shared404BB0.
		void SetTarget(JE::Entity* entity); //4052D0; NULL retains prior targetID.
		void SetItem(DWORD entityID); //4052F0.

		BYTE completed; //Original+18.
		GameCreature* giver; //Original+1C, borrowed/refreshed every Update.
		DWORD giverID; //Original+20, captured by Start.
		JE::Entity* target; //Original+24, borrowed/refreshed every Update.
		DWORD targetID; //Original+28.
		DWORD itemID; //Original+2C, zero consumes no action points.
	};

	// Explicit factor of GameServer46DD70 case83, not another original routine.
	BYTE ReceiveGameServerGiveRequest(JE::Net::Packet& packet);
}
