#pragma once

#include "ConsoleCommands.hpp"
#include "network/Types.hpp"

namespace F3
{
	void __cdecl CommandSetGlobal(CommandArguments* arguments); //58E1B0.
	void __cdecl CommandGetGlobal(CommandArguments* arguments); //58DFD0.
	void __cdecl CommandStartDialogue(CommandArguments* arguments); //58A7D0, shared589CB0.
	void __cdecl CommandEndDialogue(CommandArguments* arguments); //58A7E0.
	void __cdecl CommandWarp(CommandArguments* arguments); //58B880.
	void __cdecl CommandFadeIn(CommandArguments* arguments); //58BA10.
	void __cdecl CommandFadeOut(CommandArguments* arguments); //58BAA0.
	void __cdecl CommandStartMap(CommandArguments* arguments); //58DB80.
	void __cdecl CommandCreateEntity(CommandArguments* arguments); //58DDA0.
	void __cdecl CommandSetScript(CommandArguments* arguments); //58DF10.
	void __cdecl CommandToggleFOWLOS(CommandArguments* arguments); //58E340.
	void __cdecl CommandToggleWater(CommandArguments* arguments); //58E7E0.
	unsigned char ExecuteClientWorldCommand(JE::Net::UInt32 originalVA,
		CommandArguments* arguments);
}
