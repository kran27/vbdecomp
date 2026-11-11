#pragma once
#include <winsock2.h>
#include "ConsoleCommands.hpp"
#include "network/Types.hpp"

namespace F3
{
	void __cdecl CommandMemoryPoolTest(CommandArguments* arguments); //589580.
	bool ExecuteClientMemoryCommand(JE::Net::UInt32 address, CommandArguments* arguments);
	extern LARGE_INTEGER g_memoryTestFrequency; //Original70C578, zero/lazy QPF cache.
	extern int g_memoryTestCounter; //Original6FF980, initialized42 and shared by test values.
#if defined(_MSC_VER) && _MSC_VER == 1300
	typedef char MemoryTestFrequencyOriginalSize8[(sizeof(g_memoryTestFrequency) == 8) ? 1 : -1];
	typedef char MemoryTestCounterOriginalSize4[(sizeof(g_memoryTestCounter) == 4) ? 1 : -1];
#endif
}
