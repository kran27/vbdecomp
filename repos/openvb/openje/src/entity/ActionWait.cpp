#include "ActionWait.hpp"

namespace JE
{
	ActionWait::ActionWait() : completed(0), startTime(0), duration(0) { kind = 1; }
	ActionWait::~ActionWait() {}
	BYTE ActionWait::IsComplete() const { return completed; }
	void ActionWait::Cancel() {} //404B70.
	int ActionWait::Start(DWORD time) { startTime = time; return 0; }
	int ActionWait::Update(DWORD time, DWORD)
	{
		if (time > startTime+duration) completed = 1;
		return 0;
	}
	int ActionWait::Finish(DWORD) { return 0; }
	DWORD ActionWait::SetDuration(DWORD milliseconds) { return duration = milliseconds; }
}
