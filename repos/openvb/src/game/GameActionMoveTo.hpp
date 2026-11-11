#pragma once

#include "entity/ActionMoveTo.hpp"

namespace F3
{
	class GameActionMoveTo : public JE::ActionMoveTo
	{
	public:
		GameActionMoveTo(); //4056F0.
		virtual ~GameActionMoveTo(); //405710.
		virtual float GetSpeed(); //405720; truncate game speed before its positive test.
		virtual void GetWalkAnimation(DWORD& group, DWORD& file); //405770, group434.
		virtual void GetIdleAnimation(DWORD& group, DWORD& file); //405790, group416.
		virtual void StartMoving(); //4057B0, walk/limp events.
		virtual void StopMoving(); //406780, event1.
		virtual void OnMoved(); //4060B0, actual occupied-grid policy.
	};
}
