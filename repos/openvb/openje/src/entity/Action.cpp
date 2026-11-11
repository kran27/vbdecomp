#include "Action.hpp"
#include "Entity.hpp"

namespace JE
{
	int Action::Process(DWORD time, DWORD budget)
	{
		if (!starting && IsComplete()) return 0;
		if (starting)
		{
			starting = 0;
			Start(time);
		}
		int used = Update(time, budget);
		if (IsComplete()) Finish(time);
		return used;
	}

	ActionIdle::ActionIdle() : completed(0) { kind = 1; }
	BYTE ActionIdle::IsComplete() const { return completed; }
	void ActionIdle::Cancel() {} // Original shared404B70.
	int ActionIdle::Start(DWORD) { return 0; }
	int ActionIdle::Update(DWORD, DWORD)
	{
		completed = 1;
		return 0;
	}
	int ActionIdle::Finish(DWORD) { return 0; }

	ActionAnim::ActionAnim() : group(0), file(0), loops(0), reverse(0), completed(0), deadline(0)
	{
		kind = 1;
	}
	BYTE ActionAnim::IsComplete() const { return completed; }
	void ActionAnim::Cancel()
	{
		owner->animationEnabled = 1;
		completed = 1;
	}
	int ActionAnim::Start(DWORD time)
	{
		Entity* entity = owner;
		entity->PlayAnimation(group, file, loops, reverse, 0);
		entity->animationEnabled = 0;
		const double duration = entity->GetAnimationDuration();
		deadline = time - DWORD(static_cast<__int64>((int(loops) + 1) * duration * -1000.0));
		return 0;
	}
	int ActionAnim::Update(DWORD time, DWORD)
	{
		if (time >= deadline) completed = 1;
		return 0;
	}
	int ActionAnim::Finish(DWORD)
	{
		owner->animationEnabled = 1;
		return 0;
	}
}
