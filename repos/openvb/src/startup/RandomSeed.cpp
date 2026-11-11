// Copyright 2024 OpenJE
#include "RandomSeed.hpp"

namespace F3
{
	// Original globals: 32-bit random state at 0x707C60, initialized byte at 0x707C64.
	unsigned long g_nRandomState = 0;
	bool g_bIsRandomStateInitialized = false;

	// 0x48CCA0; evidence: docs/functions/F3-0048cca0-SetStartupTime.md.
	int SetStartupTime(int nSeed)
	{
		g_nRandomState = static_cast<unsigned long>(nSeed);
		g_bIsRandomStateInitialized = true;
		return nSeed;
	}
}
