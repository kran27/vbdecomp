// Copyright 2024 OpenJE
#include "Random.hpp"
#include "Time32.hpp"

namespace F3
{
	// 0x48CCC0.
	unsigned int __cdecl RandomNext()
	{
		typedef char RandomStateWidthsRequired[(sizeof(g_nRandomState) == 4
			&& sizeof(g_bIsRandomStateInitialized) == 1) ? 1 : -1];
		// Read the original byte truth directly, including noncanonical nonzero bytes.
		const unsigned char nInitialized =
			*reinterpret_cast<const unsigned char*>(&g_bIsRandomStateInitialized);
		if (nInitialized == 0)
		{
			// The original calls 32-bit _time(NULL), not the millisecond timer.
			// Use the selected real compiler runtime's 32-bit time API.
			const int nSeed = static_cast<int>(::time(NULL));
			SetStartupTime(nSeed);
		}
		g_nRandomState = g_nRandomState * 1664525UL + 1013904223UL;
		return static_cast<unsigned int>((g_nRandomState >> 8) & 0x7FFFUL);
	}
}
