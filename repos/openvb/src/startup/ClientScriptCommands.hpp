#pragma once

#include "ConsoleCommands.hpp"
#include "network/Types.hpp"

namespace F3
{
	void __cdecl CommandDisableScripts(CommandArguments* arguments); //58E4A0; actual required DebugWindow writer.
	void __cdecl CommandEnableScripts(CommandArguments* arguments); //58E5B0; actual required DebugWindow writer.
	void __cdecl CommandOneScriptPass(CommandArguments* arguments); //58E6C0; actual required DebugWindow writer.

	//One indicates that the callback identity was handled, not an original return.
	unsigned char ExecuteClientScriptCommand(JE::Net::UInt32 originalVA, CommandArguments* arguments);
}
