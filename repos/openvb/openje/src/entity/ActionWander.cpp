#include <winsock2.h>
#include "ActionWander.hpp"
#include "ActionMoveTo.hpp"
#include "ActionWait.hpp"
#include "Entity.hpp"
#include "World.hpp"
#include "startup/NetworkComponent.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "model/CommonException.hpp"
#include <math.h>
#include <stdlib.h>

namespace F3 { unsigned int __cdecl RandomNext(); } //Shared original48CCC0 owner.

namespace
{
	double RandomRange(float first, float last) //Original48CD30, real CRT fmod.
	{
		if (first == last) return first;
		if (last < first) { float swap = last; last = first; first = swap; }
		return fmod(F3::RandomNext(), double(last) - first + 1.0) + first;
	}
}

namespace JE
{
	ActionWander::ActionWander() : upperLeft(0,0,0), lowerRight(0,0,0),
		maximumWait(6000), repetitions(0), destination(0,0,0)
	{
		kind = 1;
		//The original also sets starting1; Action owns that constructor input.
	}
	BYTE ActionWander::IsComplete() const { return 1; }
	void ActionWander::Cancel() {}
	int ActionWander::Finish(DWORD) { return 0; }
	void ActionWander::SetBounds(const D3DXVECTOR3& first, const D3DXVECTOR3& last)
	{
		upperLeft = first;
		lowerRight = last;
	}
	void ActionWander::ChooseDestination()
	{
		float x;
		//5A285E/5A2895 treat unordered bounds like the non-less branch. The
		//decompiler's >= spelling changes that original floating-point policy.
		if (lowerRight.x < upperLeft.x)
		{
			x = upperLeft.x;
			LogDebugString("WARNING: Invalid rect provided to wander action.\n");
		}
		else x = float(RandomRange(upperLeft.x, lowerRight.x));
		double z;
		if (upperLeft.z < lowerRight.z)
		{
			//The original copies this float before logging and reloads it after.
			const float fallback = upperLeft.z;
			LogDebugString("WARNING: Invalid rect provided to wander action.\n");
			z = fallback;
		}
		else z = RandomRange(upperLeft.z, lowerRight.z);
		destination.z = float(z);
		destination.x = x;
		destination.y = 0.0f; //5A28C8..5A28CE publishes z, x, then y.
	}
	int ActionWander::Start(DWORD) { ChooseDestination(); return 0; }
	int ActionWander::Update(DWORD, DWORD)
	{
		ActionWait* wait = static_cast<ActionWait*>(g_World->NewAction(5));
		wait->SetDuration(DWORD(rand()) % (maximumWait - 1000) + 1000);
		owner->QueueAction(wait);
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		ActionMoveTo* move = static_cast<ActionMoveTo*>(g_World->NewAction(2));
		move->destination = destination;
		owner->QueueAction(move);
		if (repetitions != 1)
		{
			ActionWander* next = static_cast<ActionWander*>(g_World->NewAction(4));
			next->SetBounds(upperLeft,lowerRight);
			if (repetitions) --repetitions;
			next->repetitions = repetitions;
			owner->QueueAction(next);
		}
		return 1;
	}
}
