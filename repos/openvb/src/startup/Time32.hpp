// Real compiler-runtime 32-bit time API for the proprietary random generator.
#pragma once
#include <time.h>
namespace F3
{
	typedef time_t Time32Value;
	typedef char Time32WidthRequired[sizeof(Time32Value) == 4 ? 1 : -1];
	Time32Value __cdecl Time32(Time32Value*);
}
