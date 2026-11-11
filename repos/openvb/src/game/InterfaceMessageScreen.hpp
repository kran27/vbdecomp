#pragma once

#include <windows.h>
#include <string>
#include "gui/Interface.hpp"

namespace GUI { class Label; class Picture; }

namespace F3
{
	//Descriptive partial owner; unknown original field0 is omitted.
	//No native row layout, construction or ownership is established.
	class InterfaceMessageRow
	{
	public:
		void SetStringReference(DWORD reference); //433710, optional Label.
		void LoadTexture(const std::string& filename); //433730, optional Picture.

	private:
		InterfaceMessageRow();
		~InterfaceMessageRow();
		InterfaceMessageRow(const InterfaceMessageRow&);
		InterfaceMessageRow& operator=(const InterfaceMessageRow&);
		::GUI::Label* label; //Original+4, borrowed.
		::GUI::Picture* picture; //Original+8, borrowed.
	};

	//The same auxiliary+30 pointer reaches Interface::SetHidden without adjustment.
	//This zero-offset prefix is a partial source projection, not unique ancestry.
	//Unknown storage between Interface and original row+5C remains omitted.
	//Private undefined lifetime prevents construction or publication of this partial.
	class InterfaceMessageScreen : public ::GUI::Interface
	{
	public:
		void SetMessage(DWORD reference, const char* filename); //433840.

	private:
		InterfaceMessageScreen();
		~InterfaceMessageScreen();
		InterfaceMessageScreen(const InterfaceMessageScreen&);
		InterfaceMessageScreen& operator=(const InterfaceMessageScreen&);
		InterfaceMessageRow* row; //Original+5C, optional borrowed owner.
	};
}
