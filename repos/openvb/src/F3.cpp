// Copyright 2024 OpenJE

#include "startup/MainServices.hpp"

#include "startup/ConsoleCommands.hpp"
#include "startup/EngineRegistry.hpp"
#include "startup/MessageLoop.hpp"

namespace F3
{
	// 0x5ACE00. A departing phase does not run any subsequent phase or cleanup.
	int Main()
	{
		Startup();
		RegisterConsoleCommand("quit", &QuitCommand);
		while (ProcessMessagesAndUpdateTime())
			UpdateComponents();
		Shutdown();
		return 1;
	}

}
