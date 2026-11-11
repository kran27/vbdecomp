#pragma once

#include <string>
#include <stdexcept>
#include <vector>

namespace JE
{
	//Reconstruction diagnostic only. Its command body remains unrecovered.
	class UnrecoveredConsoleCommand : public std::logic_error
	{
	public:
		explicit UnrecoveredConsoleCommand(unsigned long address);
		unsigned long originalAddress;
	};

	//Original engine RTTI name Processor. Real host containers, no VC7 overlay.
	class ConsoleCommandProcessor
	{
	public:
		ConsoleCommandProcessor(); //580AC0, normal empty command-string vector.
		virtual ~ConsoleCommandProcessor(); //580A70, destroys queued strings/storage.
		virtual void Initialize(); //Original+04, shared404B70 no-op.
		virtual void Cleanup(); //Original+08, shared404B70 no-op.
		void Queue(const char* command); //580D10, copies/appends; no synchronous dispatch.
		void Dispatch(); //580BA0, registry callback then front erase.
		std::vector<std::string> pending; //Original+04, owning queued command copies.
	private:
		void AppendCommandArguments(char* text,std::vector<char*>& arguments); //580B00, unused native receiver.
	};
}
