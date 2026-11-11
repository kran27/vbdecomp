#pragma once
#include <windows.h>

namespace F3
{
	LONG CombatRandomRange(LONG first, LONG last); //48CD00, same-bound calls do not consume RNG.
}
