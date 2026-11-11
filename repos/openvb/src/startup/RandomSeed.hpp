// Copyright 2024 OpenJE
#pragma once

namespace F3
{
	extern unsigned long g_nRandomState;
	extern bool g_bIsRandomStateInitialized;

	// Legacy name: this seeds mutable random state, not an immutable startup clock.
	int SetStartupTime(int nSeed);
}
