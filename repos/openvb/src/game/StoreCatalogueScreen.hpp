#pragma once

#include <windows.h>
#include <list>
#include <string>
#include "gui/Interface.hpp"

namespace GUI { class Window; class ScrollBar; class Label; class Picture; }

namespace F3
{
	//Descriptive name for the complete record constructed at4571F0.
	class StoreCatalogueEntry
	{
	public:
		StoreCatalogueEntry();
		~StoreCatalogueEntry() {} //Shared5A48B0 only destroys the real string.
		DWORD value0; //Original+0, first Label string reference.
		DWORD value4; //+4, second Label string reference.
		DWORD value8; //+8, formatted as signed decimal.
		DWORD valueC; //+C, initialized to0; semantic role unresolved.
		std::string texture; //+10.

	private:
		StoreCatalogueEntry(const StoreCatalogueEntry&);
		StoreCatalogueEntry& operator=(const StoreCatalogueEntry&);
	};

	struct StoreCatalogueRow
	{
		::GUI::Window* container;
		::GUI::Label* firstReference;
		::GUI::Label* number;
		::GUI::Label* secondReference;
		::GUI::Picture* picture;
		StoreCatalogueEntry* entry; //Borrowed; hiding/clearing does not reset it.
	};

	//Observed GameInterfaces auxiliary+3C methods; original class name unknown.
	//Native callers pass this pointer directly to Interface::SetHidden. The
	//ordinary Interface prefix is observed; the unknown+34 member is still omitted,
	//so later fields and the complete screen hierarchy/layout remain unclosed.
	//Undefined private lifetime prevents accidental construction/publication.
	class StoreCatalogueScreen : public ::GUI::Interface
	{
	public:
		void ClearEntries(); //456EE0, payload destruction/free before node clear.
		void UpdateRows(); //457010, eight groups, raw scrollbar offset.
		void BuildExamples(); //457260, actual three hardcoded entries.
		void Configure(const char* filename); //457440, filename is ignored.

	private:
		StoreCatalogueScreen();
		~StoreCatalogueScreen();
		StoreCatalogueScreen(const StoreCatalogueScreen&);
		StoreCatalogueScreen& operator=(const StoreCatalogueScreen&);
		::GUI::ScrollBar* scrollbar; //Original+38; unknown+34 member still omitted.
		StoreCatalogueRow rows[8]; //+3C..FB, stride18h; required borrowed children.
		std::list<StoreCatalogueEntry*> entries; //+FC, owns payloads and list nodes.
	};
}
