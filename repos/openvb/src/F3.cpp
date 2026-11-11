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
		MainServices::Startup();
		MainServices::RegisterQuitCommand();
		while (MainServices::ProcessMessagesAndUpdateTime())
			MainServices::UpdateComponents();
		MainServices::Shutdown();
		return 1;
	}

	namespace MainServices
	{
		void Startup() { F3::Startup(); }
		void RegisterQuitCommand() { RegisterConsoleCommand("quit", &QuitCommand); }
		unsigned char ProcessMessagesAndUpdateTime()
		{
			return static_cast<unsigned char>(F3::ProcessMessagesAndUpdateTime());
		}
		void UpdateComponents() { F3::UpdateComponents(); }
		void Shutdown() { F3::Shutdown(); }
	}
}
