#pragma once

#include <windows.h>

namespace F3
{
	//Explicit435E58..435EE0 selection-to-action factor. GUI event/type/disabled
	//checks and menu closing are outside this entry; no skill result is supplied.
	void QueueClientSkillSelection(const DWORD& actorID, const DWORD& targetID, const DWORD& skill);
}
