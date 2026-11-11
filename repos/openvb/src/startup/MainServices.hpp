#pragma once

namespace F3
{
	int Main(); // 0x5ACE00

	// Production forwards to recovered game routines. Comparisons substitute
	// these boundaries without starting a game process or a Win32 GUI.
	namespace MainServices
	{
		void Startup();
		void RegisterQuitCommand();
		unsigned char ProcessMessagesAndUpdateTime();
		void UpdateComponents();
		void Shutdown();
	}
}
