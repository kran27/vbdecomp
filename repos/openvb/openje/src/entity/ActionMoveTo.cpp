#include "ActionMoveTo.hpp"
#include "Entity.hpp"
#include "EntityEnvironment.hpp"
#include "PathManager.hpp"
#include "World.hpp"
#include "graphics/GfxVector3.hpp"
#include "model/CommonException.hpp"
#include <math.h>
#include <stdexcept>

namespace
{
	double Distance(const D3DXVECTOR3& first, const D3DXVECTOR3& second) //401070.
	{
		const double x = double(first.x) - second.x, y = double(first.y) - second.y;
		const double z = double(first.z) - second.z;
		return sqrt(x*x + z*z + y*y);
	}
}

namespace JE
{
	ActionMoveTo::ActionMoveTo() : request(NULL), target(NULL), targetID(0),
		destination(0,0,0), stopDistance(5), flag34(0), movingStarted(0),
		reachability(0), checkReachability(0), completed(0)
	{
		kind = 1;
		//Original leaves lastStep/lastRepath and Action minimumBudget unwritten.
	}
	ActionMoveTo::~ActionMoveTo()
	{
		if (request)
		{
			if (!request->status)
			{
				IWorld* world = GetWorldServerInstance();
				PathRequest* pending = request;
				world->GetPathManager()->Cancel(pending);
			}
			if (request) delete request;
			request = NULL;
		}
	}
	BYTE ActionMoveTo::IsComplete() const { return completed; }
	void ActionMoveTo::Cancel() { owner->moving = 0; }
	void ActionMoveTo::SetTarget(Entity* entity)
	{
		target = entity;
		if (entity) targetID = entity->entityID;
	}
	void ActionMoveTo::SetCompleted(BYTE value)
	{
		completed = value;
		owner->moving = value == 0;
	}
	float ActionMoveTo::GetSpeed() { return 5.0f; }
	void ActionMoveTo::GetWalkAnimation(DWORD& group, DWORD& file)
	{ group = DWORD(-1); file = 0; }
	void ActionMoveTo::GetIdleAnimation(DWORD& group, DWORD& file)
	{ group = DWORD(-1); file = 0; }
	void ActionMoveTo::StartMoving()
	{
		DWORD group, file;
		GetWalkAnimation(group,file);
		if (group != DWORD(-1)) owner->PlayAnimation(group,file,0,0,0);
	}
	void ActionMoveTo::StopMoving()
	{
		DWORD group, file;
		GetIdleAnimation(group,file);
		if (group != DWORD(-1)) owner->PlayAnimation(group,file,0,0,0);
	}
	void ActionMoveTo::OnMoved() {}
	int ActionMoveTo::Start(DWORD)
	{
		target = GetEntityPtr(targetID);
		if (target)
		{
			if (target->mapHandle != owner->mapHandle)
			{ SetCompleted(1); return 0; }
			destination = target->position;
		}
		if (!request)
		{
			request = new PathRequest(owner,destination,0,0,0);
			IWorld* world = GetWorldServerInstance();
			PathRequest* pending = request;
			world->GetPathManager()->RequestPath(pending);
		}
		return 0;
	}
	int ActionMoveTo::Finish(DWORD)
	{
		if (request) { delete request; request = NULL; }
		return 0;
	}
	int ActionMoveTo::Update(DWORD time, DWORD)
	{
		if (!request)
		{
			owner->OnMoveFailed(time);
			SetCompleted(1);
			return 0;
		}
		if (!request->route)
		{
			if (request->status != 2) return 0;
			StopMoving(); owner->OnMoveFailed(time); SetCompleted(1);
			return 0;
		}
		if (request->route->empty())
		{
			StopMoving(); owner->OnMoveArrived(time); SetCompleted(1);
			lastStep = time;
			return 0;
		}
		if (!movingStarted)
		{
			movingStarted = 1;
			owner->moving = 1;
			lastStep = lastRepath = time;
			StartMoving();
		}
		if (target)
		{
			if (target->mapHandle != owner->mapHandle)
			{
				StopMoving(); owner->OnMoveFailed(time); SetCompleted(1);
				return 0;
			}
			if (Distance(target->position,destination) > 20.0)
			{
				if (DWORD(time-lastRepath) >= 2000)
				{
					lastRepath = time;
					if (request) { delete request; request = NULL; }
					destination = target->position;
					request = new PathRequest(owner,destination,0,0,0);
					IWorld* world = GetWorldServerInstance();
					PathRequest* pending = request;
					world->GetPathManager()->RequestPath(pending);
					lastStep = time;
					return 0;
				}
			}
			else if (Distance(target->position,owner->position) <= stopDistance)
			{
				if (checkReachability && owner->GetLOS(target) != reachability) return 0;
				StopMoving(); owner->OnMoveArrived(time); SetCompleted(1);
				return 0;
			}
		}
		D3DXVECTOR3 point = request->route->front();
		D3DXVECTOR3 position = owner->position;
		position.y = point.y;
		float distance = static_cast<float>(Distance(position,point));
		//55B8B2 uses the original float32 millisecond scale. The first gate
		//tests the unrounded product after separately storing remaining.
		const double movement = GetSpeed() * (DWORD(time-lastStep) * double(0.001f));
		float remaining = static_cast<float>(movement);
		if (movement > 0.0)
		{
			for (;;)
			{
				//55B8DD tests C0|C3: only ordered greater selects interpolation.
				//An unordered segment distance consumes the point, like equality.
				if (distance > remaining)
				{
					Gfx_Vector3f direction;
					direction.value[0] = point.x - position.x;
					direction.value[1] = point.y - position.y;
					direction.value[2] = point.z - position.z;
					//55B9DC/9E8/9F4 stores precede unchecked401030 normalization.
					direction.NormalizeDirection();
					D3DXVECTOR3 rotation = owner->rotation;
					rotation.x = static_cast<float>(acos(direction.value[1]*0.0 + direction.value[0]*0.0
						+ direction.value[2]) * 57.295776f);
					if (direction.value[0] < 0.0f) rotation.x = -rotation.x;
					owner->SetRotation(rotation);
					//55BA86 rounds the Z product before the three position stores.
					//X/Y keep their product precision through the addition.
					const volatile float stepZ = float(double(direction.value[2]) * remaining);
					position.x = float(double(direction.value[0]) * remaining + position.x);
					position.y = float(double(direction.value[1]) * remaining + position.y);
					position.z = float(double(stepZ) + position.z);
					break;
				}
				position = point;
				request->route->pop_front();
				remaining = float(double(remaining) - distance);
				if (request->route->empty()) break;
				//The next point and its Z/Y/X distance are read before testing
				//the remaining step, including an exactly exhausted step.
				point = request->route->front();
				const double x = double(position.x) - point.x;
				const double y = double(position.y) - point.y;
				const double z = double(position.z) - point.z;
				distance = float(sqrt(z*z + y*y + x*x));
				if (!(remaining > 0.0f)) break;
			}
		}
		position.y = request->owner->position.y;
		request->owner->SetPosition(position);
		if (request->owner->enabled == 1) request->owner->SnapToFloor();
		OnMoved();
		lastStep = time;
		return 0;
	}
}
