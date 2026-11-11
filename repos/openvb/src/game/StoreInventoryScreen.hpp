#pragma once

#include <windows.h>
#include <list>
#include <string>
#include "gui/Interface.hpp"

namespace GUI { class Window; class ScrollBar; class Label; class Picture; }

namespace F3
{
	class StoreInventoryScreen;
	//Descriptive partial record, not IScript or CStoreItem. Unknown storage,
	//including original+4, is omitted; no native layout/producer is claimed.
	class StoreScreenEntry
	{
		friend class StoreInventoryScreen;
		DWORD value0; //Original+0, accumulated with wrapping DWORD arithmetic.
		std::string texture; //Original+8; shared5A48E0 only destroys this string.
		StoreScreenEntry(); //Actual allocation/construction remains unknown.
		~StoreScreenEntry() {} //Real string lifetime only; not a recovered library body.
		StoreScreenEntry(const StoreScreenEntry&);
		StoreScreenEntry& operator=(const StoreScreenEntry&);
	};

	//Observed Store auxiliary+40 methods. Native callers use a vbptr at0 and
	//vbtable+4 to reach Interface; own members begin at+4. This minimal virtual
	//base projection does not identify intermediate bases or the full layout.
	//Undefined private construction/destruction prevent accidental publication.
	class StoreInventoryScreen : virtual public ::GUI::Interface
	{
	public:
		void PlaceForStore(BYTE active); //423E20, required root; RET4.
		void ClearFirstEntries(); //423E50, record destruction/free before node clear.
		void ClearSecondEntries(); //423EC0.
		void UpdateFirstPictures(); //423F40, eight slots, raw scrollbar offset.
		void UpdateSecondPictures(); //424140.
		void UpdateFirstTotal(); //424340, signed decimal formatting of wrapped total.
		void Refresh(); //424520, includes actual shared second-total tail424430.

	private:
		void UpdateSecondTotal(); //424430; real out-of-line tail owned by Refresh424520.
		StoreInventoryScreen();
		~StoreInventoryScreen();
		StoreInventoryScreen(const StoreInventoryScreen&);
		StoreInventoryScreen& operator=(const StoreInventoryScreen&);
		::GUI::Window* rootWindow; //Original+4, required borrowed child.
		::GUI::ScrollBar* firstScrollbar; //+8.
		::GUI::ScrollBar* secondScrollbar; //+C.
		::GUI::Label* firstTotal; //+10, optional borrowed child.
		::GUI::Label* secondTotal; //+14.
		std::list<StoreScreenEntry*> firstEntries; //+18, owns payloads and list nodes.
		std::list<StoreScreenEntry*> secondEntries; //+24.
		::GUI::Picture* secondPictures[8]; //+30..4C, required borrowed children.
		::GUI::Picture* firstPictures[8]; //+50..6C.
	};
}
