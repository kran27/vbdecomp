#pragma once

#include "ConsoleCommands.hpp"
#include "network/Types.hpp"

namespace F3
{
	void __cdecl CommandUseSkill(CommandArguments* arguments); //45EE30.
	unsigned char ExecuteClientUseSkillCommand(JE::Net::UInt32 originalVA,
		CommandArguments* arguments);
}
