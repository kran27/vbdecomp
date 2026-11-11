#pragma once
#include <windows.h>

namespace F3
{
	namespace ScriptDebugState
	{
		extern char debugText[512]; // Original70CB20, one shared formatting buffer.
		extern DWORD debugLength; // Original70CD20, zero-initialized counter.
	}
}
