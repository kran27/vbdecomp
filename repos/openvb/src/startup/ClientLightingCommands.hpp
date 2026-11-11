#pragma once

#include "ConsoleCommands.hpp"
#include "network/Types.hpp"

namespace F3
{
	void __cdecl CommandToggleLighting(CommandArguments* arguments); //5882C0; actual shared587880 body.
	//Handled marker for the real callback identity, not the original HRESULT.
	unsigned char ExecuteClientLightingCommand(JE::Net::UInt32 originalVA, CommandArguments* arguments);
}
