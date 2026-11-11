#pragma once

#include <windows.h>
#include <string>
#include <vector>

namespace GUI { class Window; class Label; struct Event; }

namespace F3
{
	struct CharacterGenerationTrait
	{
		const char* buttonName;
		const char* labelName;
		LONG id;
	};
	extern CharacterGenerationTrait CharacterGenerationTraits[16]; //6F28F8, writable authored rows.

	struct CharacterGenerationChoice
	{
		DWORD reference;
		LONG value;
		DWORD value8, valueC;
		std::string text;
	};
	struct CharacterGenerationChoiceRow
	{
		::GUI::Window* button;
		::GUI::Label* label;
	};

	//Actual nonvirtual68-byte proprietary owner42A6A0. Standard vectors own
	//their entries/strings and storage; all bound GUI widgets are borrowed.
	class CharacterGenerationChoices
	{
	public:
		CharacterGenerationChoices(); //42A6A0.
		~CharacterGenerationChoices(); //42A710.
		void Clear(); //42A770 ->429B70.
		void SetRowCount(LONG count); //42A740.
		void BindRow(LONG index, ::GUI::Window* button, ::GUI::Label* label); //425200.
		void Add(DWORD reference, LONG value, DWORD value8, DWORD valueC,
			const std::string& text); //42CCF0; actual vector value append42CC60.
		BYTE SetSelection(LONG index); //424DF0.
		BYTE SetFirstVisible(LONG index); //424E50.
		LONG GetFirstVisible() const; //424A10.
		void EnsureSelectionVisible(); //Shared tails of425000..425120.
		BYTE Process(::GUI::Window* window, ::GUI::Event* event); //424EC0.
		void Update(); //425940 dirty gate ->425240.
		void BindArrows(::GUI::Window* previous, ::GUI::Window* next); //Shared4AB6C0.

		std::vector<CharacterGenerationChoice> entries; //Original+00,44-byte values.
		std::vector<CharacterGenerationChoiceRow> rows; //+10,8-byte borrowed pairs.
		::GUI::Window* previousButton; //+20.
		::GUI::Window* nextButton; //+24.
		LONG selected, firstVisible; //+28/+2C, initial-1/0.
		BYTE dirty; //+30; constructor leaves padding31..33 unwritten.
		DWORD pressTime, repeatTime, holdDelay, repeatDelay; //+34..40.
	private:
		void RebuildRows(); //425240.
	};
}
