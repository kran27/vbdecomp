#pragma once

#include "entity/Action.hpp"

namespace F3
{
	class GameCreature;
	class GameWeapon;
	class GameAmmo;
	class GameActionReload : public JE::Action
	{
	public:
		GameActionReload(); //4071C0.
		virtual ~GameActionReload() {}
		virtual BYTE IsComplete() const; //407200.
		virtual void Cancel(); //404B70 empty hook.
		virtual int Start(DWORD time); //407210, capture supplied weapon/stack IDs.
		virtual int Update(DWORD time, DWORD budget); //407270.
		virtual int Finish(DWORD time); //404BB0.
		GameCreature* creature;
		GameWeapon* weapon;
		DWORD weaponID;
		GameAmmo* stack;
		DWORD stackID;
		BYTE secondary, completed, flag2E;
		DWORD cost;
		BYTE firstUpdate;
		DWORD animationEnd;
	private:
		DWORD PlayReloadAnimation(); //407250; returns duration or1, not time+duration.
	};
}
