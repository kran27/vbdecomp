#include "ConsoleCommands.hpp"
#include "Window.hpp"
#include "diagnostics/DebugOutput.hpp"

#include <cstring>

namespace F3
{
	ConsoleCommands g_ConsoleCommands;

	unsigned char CommandNameLess::operator()(const std::string& sLeft,
		const std::string& sRight) const
	{
		return _stricmp(sLeft.c_str(), sRight.c_str()) < 0;
	}

	// 0x59F5D0. Null callbacks still reserve their names against duplicates.
	void RegisterConsoleCommand(const char* pszName, CommandCallback pCallback)
	{
		std::string sName(pszName);
		ConsoleCommands::const_iterator found = g_ConsoleCommands.find(sName);
		if (found != g_ConsoleCommands.end())
			JE::FatalError("Command %s already registered.", pszName);
		g_ConsoleCommands[sName] = pCallback;
	}

	// 0x59EC80 returns the stored callback; it does not execute a command.
	CommandCallback FindConsoleCommand(const std::string& sName)
	{
		ConsoleCommands::const_iterator found = g_ConsoleCommands.find(sName);
		return found == g_ConsoleCommands.end() ? NULL : found->second;
	}

	// 0x59F030 preserves the result across temporary string destruction.
	CommandCallback FindConsoleCommand(const char* pszName)
	{
		std::string name(pszName);
		return FindConsoleCommand(name);
	}

	void __cdecl QuitCommand(CommandArguments*)
	{
		Quit();
	}
}
