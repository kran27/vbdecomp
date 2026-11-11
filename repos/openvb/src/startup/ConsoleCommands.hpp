#pragma once

#include <map>
#include <string>
#include <vector>
#include "network/CommandProcessor.hpp"

namespace F3
{
	// 0x580B00 appends char* tokens; 0x580C4B passes that vector by pointer.
	// Native callers use the supplied VC7 vector.
	typedef std::vector<char*> CommandArguments;
	typedef void (__cdecl *CommandCallback)(CommandArguments* pArguments);

	struct CommandNameLess
	{
		unsigned char operator()(const std::string& sLeft, const std::string& sRight) const;
	};
	typedef std::map<std::string, CommandCallback, CommandNameLess> ConsoleCommands;
	extern ConsoleCommands g_ConsoleCommands; // original 0x70C668

	void RegisterConsoleCommand(const char* pszName, CommandCallback pCallback); // 0x59F5D0
	CommandCallback FindConsoleCommand(const std::string& sName); // 0x59EC80
	CommandCallback FindConsoleCommand(const char* pszName); // 0x59F030
	void __cdecl QuitCommand(CommandArguments* pArguments); // 0x5ACD50
}

