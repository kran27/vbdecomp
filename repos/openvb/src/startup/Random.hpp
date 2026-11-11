// Copyright 2024 OpenJE
#pragma once

#include "Time32.hpp"
#include "RandomSeed.hpp"

namespace F3
{
	// Descriptive name for original 0x48CCC0; returns a 15-bit random value.
	unsigned int __cdecl RandomNext();
	float __cdecl RandomFloat(float low, float high); //48CD30, inclusive-width fmod policy.
	int __cdecl RandomInteger(int low, int high); //48CD00, signed bounds and unsigned modulo.
}
