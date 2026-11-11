#include "GameProjectile.hpp"
#include "entity/World.hpp"
#include "entity/ActionMoveTo.hpp"
#include "entity/Effect.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cmath>

namespace F3
{
	GameProjectile::GameProjectile()
	{
		destination.x = destination.y = destination.z = 0.0f;
		entityClass = 8;
		enabled = 0;
		moveByAction = 0;
		targetEntityID = 0;
		lastMovementTime = 0;
		shadow = 0;
		GameEntity::SetSelectable(0);
	}

	GameProjectile::~GameProjectile()
	{
		//411D50 adds no payload drop or other cleanup before GameEntity lifetime.
	}

	void GameProjectile::CopyFrom(JE::Entity* source)
	{
		GameProjectile* projectile = dynamic_cast<GameProjectile*>(source);
		if (projectile)
		{
			moveByAction = projectile->moveByAction;
			destination = projectile->destination;
			targetEntityID = projectile->targetEntityID;
		}
		//Original does not copy the movement timestamp.
		GameEntity::CopyFrom(source);
	}

	void GameProjectile::OnActivate(DWORD time)
	{
		GameEntity::OnActivate(time);
		if (moveByAction)
		{
			JE::IWorld* world = JE::g_World;
			JE::ActionMoveTo* action = static_cast<JE::ActionMoveTo*>(world->NewAction(2));
			if (targetEntityID)
			{
				JE::Entity* target = JE::GetEntityPtr(targetEntityID);
				if (!target)
				{
					Destroy();
					return; //Original leaks the new, unqueued action on this branch.
				}
				action->SetTarget(target);
			}
			else action->destination = destination;
			QueueAction(action);
		}
		lastMovementTime = time;
	}

	void GameProjectile::OnMoveFailed(DWORD time)
	{
		JE::Entity::OnMoveFailed(time);
		Destroy(); //Failure does not drop the payload.
	}

	void GameProjectile::OnMoveArrived(DWORD time)
	{
		JE::Entity::OnMoveArrived(time);
		DropPayload();
		Destroy();
	}

	void GameProjectile::DropPayload()
	{
		while (effects[1].size()>0)
		{
			JE::Effect* effect = *effects[1].begin();
			effects[1].pop_front(); //Unlink before either role lookup or transfer.
			DWORD role = effect->GetRole();
			switch (role)
			{
			case 0: case 1: case 2:
			{
				//The second virtual role lookup is present in the original.
				JE::Entity* target = JE::GetEntityPtr(effect->targets[effect->GetRole()]);
				if (target) target->AddEffect(effect);
				else effect->Destroy();
				break;
			}
			case 3: break;
			default:
				JE::FatalError("GameProjectile::DropPayload() - Unknown target type.");
			}
			//Role3 is unlinked and left allocated by the original.
		}
	}

	void GameProjectile::Update(DWORD time)
	{
		JE::Entity::Update(time);
		JE::Entity* target = NULL;
		if (targetEntityID)
		{
			target = JE::GetEntityPtr(targetEntityID);
			if (!target || target->mapHandle != mapHandle)
			{
				Destroy();
				return;
			}
		}
		if (!moveByAction)
		{
			D3DXVECTOR3 next = position;
			double targetX = target ? target->position.x : destination.x;
			double targetZ = target ? target->position.z : destination.z;
			next.y = 0.0f;
			D3DXVECTOR3 angles = rotation;
			double dx = double(next.x) - targetX;
			double dz = double(next.z) - targetZ;
			float distance = float(std::sqrt(dz * dz + dx * dx));
			float directionX = float(targetX - next.x);
			double directionZ = targetZ - next.z;
			double reciprocal = 1.0 / std::sqrt(directionZ * directionZ
				+ double(directionX) * directionX);
			D3DXVECTOR3 direction(float(double(directionX) * reciprocal),
				float(0.0 * reciprocal), float(directionZ * reciprocal));
			//Preserve the original zero-distance/acos domain, including its NaNs.
			angles.x = float(std::acos(double(direction.y) * 0.0
				+ double(direction.x) * 0.0 + direction.z) * double(57.295776f));
			if (direction.x < 0.0f) angles.x = -angles.x;
			SetRotation(angles);
			double step = double(DWORD(time - lastMovementTime)) * double(0.001f) * 10.0;
			if (step < distance)
			{
				next.x = float(double(direction.x) * step + next.x);
				next.y = float(double(direction.y) * step + next.y);
				float deltaZ = float(double(direction.z) * step);
				next.z = float(double(deltaZ) + next.z);
				next.y = position.y; //Reload after the rotation child; no vertical travel.
				SetPosition(next);
				lastMovementTime = time;
				return;
			}
			DropPayload();
			Destroy();
		}
		lastMovementTime = time;
	}
}
