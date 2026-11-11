// Copyright 2024 OpenJE
#pragma once

#include <windows.h>
#include <mmsystem.h>

namespace F3
{
	extern unsigned long g_nSystemTimeMS;
	extern unsigned long g_nElapsedTimeMS;

	DWORD GetElapsedTimeMS(); //56AF50; chosen source name for the actual accumulated-clock getter.
	bool ProcessMessagesAndUpdateTime();
}

// The verification build substitutes only the original routine's API boundary.
