#pragma once

#include "ConsoleCommands.hpp"
#include "network/Types.hpp"

namespace F3
{
	void __cdecl CommandForceEntityStep(CommandArguments* arguments); //460720.
	void __cdecl CommandTestAnim(CommandArguments* arguments); //460920.
	unsigned char ExecuteClientEntityCommand(JE::Net::UInt32 originalVA,
		CommandArguments* arguments);
}
