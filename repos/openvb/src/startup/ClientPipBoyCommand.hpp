#pragma once

#include "ConsoleCommands.hpp"
#include "network/Types.hpp"

namespace F3
{
	void __cdecl CommandTogglePipBoy(CommandArguments* arguments); //45EC70.
	unsigned char ExecuteClientPipBoyCommand(JE::Net::UInt32 originalVA,
		CommandArguments* arguments);
}
