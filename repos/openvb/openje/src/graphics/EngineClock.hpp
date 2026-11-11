#pragma once

#include <windows.h>

struct Gfx_EngineClockState
{
	Gfx_EngineClockState(); //Corresponding5BFE10 zero stores only.
	DWORD deltaMilliseconds, milliseconds, secondsWord;
	float Seconds() const;
};

// The clock uses three actual scalar words, never overlaid library objects.
void Gfx_AdvanceEngineClock(DWORD& delta, DWORD& total, DWORD& secondsWord,
	DWORD milliseconds); //5BDDB0 including51DA40.
void Gfx_SetEngineClock(DWORD& total, DWORD& secondsWord, DWORD milliseconds); //5BDDF0.
float Gfx_EngineClockSeconds(DWORD secondsWord); //5C0170.
