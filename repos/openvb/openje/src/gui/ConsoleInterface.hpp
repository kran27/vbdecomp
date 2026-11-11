#pragma once

#include "gui/EngineInterface.hpp"
#include "gui/Edit.hpp"
#include <list>

namespace JE
{
	struct ConsoleLine
	{
		ConsoleLine() : kind(0) { text = ""; } //59DDA0, native post-kind empty assignment.
		std::string text;
		DWORD kind; //1 command,2 output.
	};

	class ConsoleInterface : public virtual EngineInterface
	{
	public:
		explicit ConsoleInterface(const std::string& name); //59E180.
		virtual ~ConsoleInterface(); //59DBC0, followed by actual member/base destruction.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //59EC10.
		void ClearInput(); //59DE10.
		void PreviousCommand(); //59D9F0.
		void NextCommand(); //59DEB0.
		void RefreshLines(); //59E050.
		void ClearHistory(); //59DB40.
		void Print(const char* format, ...); //59EAF0.
		LONG historyCursor;
		::GUI::Window rootWindow;
		::GUI::Edit input;
		::GUI::Label prompt, lines[10];
		std::list<ConsoleLine*> history; //Newest first; owns the real string/kind records.
	};
}
