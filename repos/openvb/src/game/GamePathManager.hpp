#pragma once

#include "entity/PathManager.hpp"

namespace F3
{
	class GamePathManager : public JE::PathManager
	{
	public:
		GamePathManager(); // 0x45BA40, factory46F9E0.
		virtual ~GamePathManager(); // 0x45BA60; additional derived virtual slot.
		virtual LONG Estimate(LONG fromX, LONG fromY, LONG toX, LONG toY); // 0x45BA70.
		virtual LONG StepCost(const JE::PathPoint& from, const JE::PathPoint& to,
			JE::PathRequest* request); // 0x45BB10.
		virtual BYTE CanTraverse(JE::PathRequest* request, D3DXVECTOR3 from,
			D3DXVECTOR3 to); // 0x45BC90.
		virtual BYTE IsBlocked(BYTE flags, JE::Entity* entity); // 0x45BA90.
	};
}
