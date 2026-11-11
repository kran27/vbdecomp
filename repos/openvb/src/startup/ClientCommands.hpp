#pragma once

#include "ConsoleCommands.hpp"
#include "network/Types.hpp"

namespace F3
{
	// Recovered callbacks bind directly. Remaining addressed fallbacks and aliases
	// retain pointer equality, including four names sharing original0x404B70.
	void __cdecl CommandToggleInventoryWindow(CommandArguments* arguments); //45EAC0, ignored argument.
	CommandCallback GetClientCommandCallback(JE::Net::UInt32 originalVA);
	void RegisterClientCommand(const char* name, JE::Net::UInt32 originalVA);

	namespace ClientCommandRegistrationServices
	{
		void Register(const char* name, CommandCallback callback);
	}
}
