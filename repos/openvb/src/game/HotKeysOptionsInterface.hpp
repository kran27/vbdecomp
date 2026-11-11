#pragma once

#include "game/GameInterface.hpp"
#include <list>
#include <string>

namespace GUI
{
	class Label;
	class Picture;
}

namespace F3
{
	//Actual128-byte formatting record445C10. Ordinary strings own their data;
	//the original frame byte's following padding is not a fourth flag owner.
	struct HotKeyWindowFormat
	{
		HotKeyWindowFormat(); //445C10.
		virtual ~HotKeyWindowFormat(); //445690.
		std::string skin, font, texture;
		LONG fontSize;
		DWORD fontFlags, textColor;
		LONG x, y, width, height;
		BYTE frameVisible;
		DWORD alignment, verticalAlignment;
	};

	//The page owns this record; the actual GUI parent graph owns its widgets.
	struct HotKeyRow
	{
		HotKeyRow(); //445B50.
		virtual ~HotKeyRow(); //445640; strings only, no widget deletion.
		::GUI::Label* keyLabel;
		::GUI::Label* descriptionLabel;
		::GUI::Picture* cell;
		BYTE editing;
		std::string display, command;
		LONG scan;
		DWORD modifiers;
	};

	class HotKeysOptionsInterface : public GameInterface
	{
	public:
		explicit HotKeysOptionsInterface(const std::string& name); //4468C0.
		virtual ~HotKeysOptionsInterface(); //445740.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //4465E0/4468B0 adjustor.
		void Commit(); //445380.
		void ReplaceCommandBinding(LONG scan, DWORD subtype, DWORD modifiers,
			const char* command); //4452B0; native member receiver and RET16.
		void PrepareForOpen(); //4461D0: EndEdit, then ReloadBindings.
		void RestoreDefaults(); //4455D0; real named bindings change immediately.
		void ReloadBindings(); //446080.
		void FormatKey(std::string& output, LONG scan, DWORD subtype, DWORD modifiers); //445D20, member RET16.
		BYTE IsEditing(); //445350.
		void BeginEdit(HotKeyRow* row); //445970.
		void EndEdit(); //445A40; RemoveModal is unconditional.
		void Remap(LONG scan, DWORD modifiers); //445E20.
		void AddRow(const std::string& command, const std::string& description); //4461E0, RET8.

		::GUI::Picture* background; //Original+04.
		std::list<HotKeyRow*> rows; //+08; original+10 is this list's size.
		HotKeyWindowFormat descriptionFormat; //+14.
		HotKeyWindowFormat cellFormat; //+94.
		HotKeyWindowFormat keyLabelFormat; //+114.
	};
}
