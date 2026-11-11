#include "GameActionOperateDoor.hpp"
#include "GameDoor.hpp"
#include "GameEntity.hpp"
#include "entity/ActionMoveTo.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include <math.h>

namespace F3
{
	GameActionOperateDoor::GameActionOperateDoor()
		: door(NULL), doorID(0), deadline(0), firstUpdate(1), completed(0)
	{
		kind = 1;
	}
	BYTE GameActionOperateDoor::IsComplete() const { return completed; }
	void GameActionOperateDoor::Cancel() {}
	int GameActionOperateDoor::Start(DWORD) { return 0; }
	int GameActionOperateDoor::Finish(DWORD) { return 0; }
	void GameActionOperateDoor::SetDoor(GameDoor* value)
	{
		door = value;
		if (value) doorID = value->entityID;
	}
	DWORD GameActionOperateDoor::StartOperating()
	{
		GameEntity* game = dynamic_cast<GameEntity*>(owner);
		if (!game) return 0;
		game->EndAnimationNow();
		return game->QueueAnimationEvent(121, 0);
	}
	void GameActionOperateDoor::StopOperating()
	{
		GameEntity* game = dynamic_cast<GameEntity*>(owner);
		if (game)
		{
			game->EndAnimationNow();
			game->QueueAnimationEvent(1, 0);
		}
	}
	int GameActionOperateDoor::Update(DWORD time, DWORD budget)
	{
		int used = 0;
		door = dynamic_cast<GameDoor*>(JE::GetEntityPtr(doorID));
		if (!door) { completed = 1; return 0; }
		if (door->mapHandle != owner->mapHandle) { completed = 1; return 0; }
		D3DXVECTOR3 from = owner->position;
		D3DXVECTOR3 to = door->position;
		double x = double(from.x) - to.x, y = double(from.y) - to.y, z = double(from.z) - to.z;
		if (sqrt(x*x + y*y + z*z) >= 5.0)
		{
			JE::ActionMoveTo* move = static_cast<JE::ActionMoveTo*>(JE::g_World->NewAction(2));
			move->SetTarget(door);
			move->stopDistance = 5;
			owner->QueueAction(move);
			GameActionOperateDoor* retry = new GameActionOperateDoor;
			retry->SetDoor(door);
			owner->QueueAction(retry);
			completed = 1;
			return 0;
		}
		if (budget < 6) return 0;
		if (firstUpdate)
		{
			firstUpdate = 0;
			D3DXVECTOR3 direction = to - from;
			// Actual401030 divides by length without a zero-vector guard.
			double reciprocal = 1.0 / sqrt(double(direction.x)*direction.x
				+ double(direction.y)*direction.y + double(direction.z)*direction.z);
			direction.x = static_cast<float>(reciprocal * direction.x);
			direction.y = static_cast<float>(reciprocal * direction.y);
			direction.z = static_cast<float>(reciprocal * direction.z);
			D3DXVECTOR3 rotation = owner->rotation;
			rotation.x = static_cast<float>(acos(direction.z) * 57.295776f);
			if (direction.x < 0.0f) rotation.x = -rotation.x;
			owner->SetRotation(rotation);
			deadline = time + StartOperating();
			used = 6;
		}
		if (deadline && time >= deadline)
		{
			door->Operate(owner);
			StopOperating();
			deadline = 0;
			completed = 1;
		}
		return used;
	}
}
