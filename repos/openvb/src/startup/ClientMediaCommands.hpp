#pragma once

#include "ConsoleCommands.hpp"
#include "network/Types.hpp"

namespace F3
{
	void __cdecl CommandMovieTest(CommandArguments* arguments); //588D00.
	bool ExecuteClientMediaCommand(JE::Net::UInt32 address, CommandArguments* arguments);
}
