#pragma once

#include "game/GameInterface.hpp"
#include <list>

namespace GUI
{
	class Label;
}

//Actual global RTTI SaveSlotOption, table65A6B4. It owns neither Label pointer.
class SaveSlotOption
{
public:
	SaveSlotOption(); //4484D0.
	virtual ~SaveSlotOption(); //4484F0; deleting entry448500.
	::GUI::Label* slotLabel; //+04, initialNULL, generated root child.
	::GUI::Label* nameLabel; //+08, initialNULL, generated root child.
};

namespace F3
{
	class OptionsLoadInterface : public GameInterface
	{
	public:
		explicit OptionsLoadInterface(const std::string& name); //448C40.
		virtual ~OptionsLoadInterface(); //4486E0.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //4485A0.
		void ClearSelection(); //448590:SelectSlot(NULL), no stored selection field.

		::GUI::Window* rootWindow; //+04, borrowed serialized root.
		::GUI::Label* informationLabels[3]; //+08,+0C,+10, borrowed.
		std::string rowSkin; //+14, defaultDefaultInterface.ini.
		std::string rowFontName; //+30, defaultarial.
		LONG rowX, rowY, rowWidth, rowHeight; //+4C,+50,+54,+58.
		LONG rowFontSize; //+5C, default10.
		DWORD rowFontFlags; //+60, default0.
		std::list<SaveSlotOption*> rows; //+64:owns records only;original head+68.
	private:
		void BuildRows(); //448900, fifteen original prototype slots.
		void SelectSlot(SaveSlotOption* selected); //448520, changes both text colors.
	};
}
