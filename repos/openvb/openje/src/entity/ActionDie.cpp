#include "startup/NetworkComponent.hpp"
#include "ActionDie.hpp"
#include "Entity.hpp"

namespace JE
{
	ActionDie::ActionDie() : completed(0) { kind=1; }
	BYTE ActionDie::IsComplete() const { return completed; }
	void ActionDie::Cancel() {}
	int ActionDie::Start(DWORD) { owner->Die(); return 0; }
	int ActionDie::Update(DWORD, DWORD) { return 0; }
	int ActionDie::Finish(DWORD) { return 0; }

	void Entity::Die() //551CA0, no state change outside server mode.
	{
		if (F3::g_pNetworkInstance->GetMode()&1) ChangeState(6);
	}
}
