#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "ConsoleInterface.hpp"
#include "gui/Event.hpp"
#include "gui/Manager.hpp"
#include "network/CommandProcessor.hpp"
#include <cstdarg>
#include <cstdio>

namespace JE
{
	ConsoleInterface::ConsoleInterface(const std::string& name)
		: EngineInterface(name), rootWindow(0), input(4), prompt(3)
	{
		historyCursor=-1;
		history.clear();
		rootWindow.Resize(1024,162);
		rootWindow.ApplySkin("DefaultInterface.ini");
		rootWindow.SetAbsolutePosition(0,0);
		rootWindow.SetName("ConsoleWindow");
		LONG y=4;
		for (::GUI::Label* current=lines;y<144;++current,y+=14)
		{
			::GUI::Label& line=*current;
			rootWindow.AddChild(&line);
			line.Resize(1016,14);
			line.ApplySkin("DefaultInterface.ini");
			line.SetFont("Lucida Console",0,10,1);
			line.SetTextColor(0xFF808080);
			line.SetText("Test Console Line");
			line.SetClientPosition(4,y);
		}
		rootWindow.AddChild(&prompt);
		prompt.Resize(12,14);
		prompt.ApplySkin("DefaultInterface.ini");
		prompt.SetFont("Lucida Console",0,10,1);
		prompt.SetTextColor(0xFF808080);
		prompt.SetText(">");
		prompt.SetClientPosition(4,144);
		rootWindow.AddChild(&input);
		input.ApplySkin("DefaultInterface.ini");
		input.SetTextFrame(0); //Actual virtual+178 ->4C03B0 ->Label4A7860.
		input.SetFont("Lucida Console",0,10,1);
		input.SetTextColor(0xFF808080);
		input.SetText("");
		input.Resize(1016,14);
		input.SetClientPosition(16,144);
		AddWindow(&rootWindow);
		RefreshLines();
	}
	ConsoleInterface::~ConsoleInterface()
	{
		::GUI::g_Manager.RemoveModal(this);
		ClearHistory();
		//Embedded windows detach from their parent/owner before the bases die.
	}
	void ConsoleInterface::ClearHistory()
	{
		for (std::list<ConsoleLine*>::iterator entry=history.begin();entry!=history.end();++entry)
			delete *entry;
		history.clear();
	}
	void ConsoleInterface::ClearInput()
	{
		historyCursor=-1;
		input.SetText("");
	}
	void ConsoleInterface::RefreshLines()
	{
		LONG row=9;
		for (std::list<ConsoleLine*>::iterator entry=history.begin();entry!=history.end() && row>=0;++entry)
			if (*entry) { lines[row].SetText((*entry)->text); --row; }
		while (row>=0) { lines[row].SetText(""); --row; }
	}
	void ConsoleInterface::PreviousCommand()
	{
		LONG index=0;
		bool clamped=false;
		++historyCursor;
		if (historyCursor>LONG(history.size())-1)
		{
			historyCursor=LONG(history.size())-1;
			clamped=true;
		}
		for (std::list<ConsoleLine*>::iterator entry=history.begin();entry!=history.end();++entry,++index)
			if (index==historyCursor)
			{
				if (*entry && (*entry)->kind==1)
				{
					input.SetText((*entry)->text);
					std::string currentText;
					input.GetText(currentText);
					const LONG length = LONG(currentText.size());
					input.SetSelection(length, length);
				}
				else if (!clamped) PreviousCommand();
			}
	}
	void ConsoleInterface::NextCommand()
	{
		LONG index=0;
		if (--historyCursor<0) { historyCursor=-1; input.SetText(""); }
		for (std::list<ConsoleLine*>::iterator entry=history.begin();entry!=history.end();++entry,++index)
			if (index==historyCursor)
			{
				if (*entry && (*entry)->kind==1)
				{
					input.SetText((*entry)->text);
					std::string currentText;
					input.GetText(currentText);
					const LONG length = LONG(currentText.size());
					input.SetSelection(length, length);
				}
				else NextCommand();
			}
	}
	void ConsoleInterface::Print(const char* format, ...)
	{
		char buffer[1024];
		va_list arguments;
		va_start(arguments,format);
		std::vsprintf(buffer,format,arguments); //Original1024-byte formatter; overflow domain unvalidated.
		va_end(arguments);
		ConsoleLine* line=new ConsoleLine;
		line->kind=2;
		line->text=buffer;
		history.push_front(line);
		RefreshLines();
	}
	int ConsoleInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (GetHidden())
			return EngineInterface::OnEvent(window,event);
		if (event->type==17 && window==&input)
		{
			switch (event->x)
			{
			case 1: case 41:
				ClearInput();
				SetHidden(1);
				::GUI::g_Manager.RemoveModal(this);
				break;
			case 28:
			{
				std::string text;
				input.GetText(text);
				if (text.size()>0)
				{
					ConsoleLine* line=new ConsoleLine;
					line->kind=1; line->text=text;
					history.push_front(line);
					static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x7C->Queue(text.c_str());
					RefreshLines();
					ClearInput();
				}
				break;
			}
			case 328: PreviousCommand(); break;
			case 336: NextCommand(); break;
			}
		}
		return EngineInterface::OnEvent(window,event);
	}
}
