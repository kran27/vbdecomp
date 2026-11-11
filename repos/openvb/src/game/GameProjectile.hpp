#pragma once

#include "GameEntity.hpp"

namespace F3
{
	class GameProjectile : public GameEntity
	{
		friend JE::Entity* CreateGameEntity(DWORD type);
	public:
		virtual void CopyFrom(JE::Entity* source); //411E90, Entity-interface thunk411F10.
		virtual void OnActivate(DWORD time); //411DD0, thunk412000.
		virtual void Update(DWORD time); //412180, thunk412410.
		virtual void OnMoveFailed(DWORD time); //411D90, thunk411F80.
		virtual void OnMoveArrived(DWORD time); //412130, thunk412400.
		void DropPayload(); //412090; transfers effects[1] through actual Entity owners.

		BYTE moveByAction; //Original+A0; nonzero selects real MoveTo action.
		D3DXVECTOR3 destination; //+A4/+A8/+AC; no target entity uses X/Z only.
		DWORD targetEntityID; //+B0, optional live lookup.
		DWORD lastMovementTime; //+B4, absolute milliseconds.
	protected:
		GameProjectile(); //411C60, complete most-derived construction of Entity/GameEntity.
		virtual ~GameProjectile(); //411D50, then normal inherited member lifetime.
	};
}
