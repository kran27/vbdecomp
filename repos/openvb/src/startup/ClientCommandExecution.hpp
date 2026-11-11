#pragma once

#include "ConsoleCommands.hpp"
#include "network/Types.hpp"

namespace F3
{
	// Called by the per-identity native adapters in ClientCommands.cpp.
	// Only recovered callbacks are executable; other identities fail explicitly.
	void ExecuteClientCommand(JE::Net::UInt32 originalVA, CommandArguments* pArguments);
}
