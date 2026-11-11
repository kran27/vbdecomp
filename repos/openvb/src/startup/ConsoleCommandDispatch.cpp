#include "network/EngineClient.hpp"
#include "gui/ConsoleInterface.hpp"
#include "ConsoleCommands.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>
#include <cstdio>
#include <stdexcept>

namespace
{
	std::string UnrecoveredMessage(unsigned long address)
	{
		char message[80];
		std::sprintf(message,"Unrecovered client console command 0x%08lX",address);
		return message;
	}

}

namespace JE
{
	void ConsoleCommandProcessor::AppendCommandArguments(char* text,std::vector<char*>& arguments) //580B00, RET8.
	{
		//61A7D1 is the supplied real CRT strtok, with exactly one delimiter:
		//spaces. Tabs/quotes are not a replacement shell grammar.
		for (text=std::strtok(text," ");text;text=std::strtok(NULL," "))
			arguments.push_back(text);
	}
	UnrecoveredConsoleCommand::UnrecoveredConsoleCommand(unsigned long address)
		: std::logic_error(UnrecoveredMessage(address)), originalAddress(address) {}
	ConsoleCommandProcessor::ConsoleCommandProcessor() {} //580AC0, vector construction.
	ConsoleCommandProcessor::~ConsoleCommandProcessor() {} //580A70, vector destruction.
	void ConsoleCommandProcessor::Initialize() {} //404B70.
	void ConsoleCommandProcessor::Cleanup() {} //404B70, does not clear queued commands.

	void ConsoleCommandProcessor::Queue(const char* command) //580D10.
	{
		const std::string copy(command);
		pending.push_back(copy); //4C60A0, actual vector<string> append.
	}

	void ConsoleCommandProcessor::Dispatch() //580BA0.
	{
		F3::CommandArguments arguments;
		char buffer[256];
		std::vector<std::string>::iterator current=pending.begin();
		while (current!=pending.end())
		{
			std::strcpy(buffer,current->c_str());
			AppendCommandArguments(buffer,arguments);
			F3::CommandCallback callback=F3::FindConsoleCommand(arguments.front());
			if (callback) callback(&arguments);
			else LogDebugString("Unknown command %s\n",current->c_str());
			pending.erase(current);
			//Keep the captured iterator and borrowed tokens across commands.
		}
	}
}
