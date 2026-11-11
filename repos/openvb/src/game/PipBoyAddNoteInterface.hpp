#pragma once

#include "game/GameInterface.hpp"

namespace JE { class AreaMap; class AreaMapNote; }
namespace F3
{
	BYTE NoteTextIsBlank(const char* text); //Fixed note whitespace predicate via real CRT; support adapter only.
	class PipBoyInterface;
	class PipBoyAddNoteInterface : public GameInterface
	{
	public:
		explicit PipBoyAddNoteInterface(const std::string& name); //44DA10.
		virtual ~PipBoyAddNoteInterface(); //44A560.
		virtual int OnEvent(::GUI::Window*, ::GUI::Event*); //453EB0, logical body453540.
		std::string GetIconTextureName(LONG index); //44B2A0.
		DWORD GetIconTexture(LONG index); //44AD50.
		void RefreshIcons(); //44DD70.
		void Open(PipBoyInterface* parent, JE::AreaMap* map, JE::AreaMapNote* target, BYTE readOnly); //44FC70.
		void SetPopupPosition(LONG x, LONG y); //44DCB0.
		PipBoyInterface* pipBoy; //+04, borrowed, initialNULL.
		JE::AreaMap* areaMap; //+08, borrowed, initialNULL.
		JE::AreaMapNote* note; //+0C, borrowed, initialNULL.
		LONG iconIndex; //+10, initial1.
	private:
		void RefreshText(); //44DB10, actual required Edit child.
	};
}
