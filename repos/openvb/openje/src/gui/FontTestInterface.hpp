#pragma once

#include "gui/EngineInterface.hpp"

namespace GUI { class Window; class Label; }

namespace JE
{
	class TextEditInterface;
	class FontTestInterface : public virtual EngineInterface
	{
	public:
		explicit FontTestInterface(const std::string& name); //5B80E0, allocation54h.
		virtual ~FontTestInterface(); //5B7760, deliberately leaves editor alive.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //5B8A10/5B8690.

		::GUI::Window* main; //04, borrowed TextWin.
		::GUI::Label* text1; //08.
		::GUI::Label* text2; //0C.
		::GUI::Label* text3; //10.
		::GUI::Label* text4; //14.
		TextEditInterface* editor; //18, new child; original destructor does not delete it.
	};

#if defined(_MSC_VER) && _MSC_VER == 1300
	typedef char FontTestOriginalExtent[(sizeof(FontTestInterface) == 0x54) ? 1 : -1];
	typedef char FontTestOriginalRoot[(offsetof(FontTestInterface, main) == 4) ? 1 : -1];
	typedef char FontTestOriginalText1[(offsetof(FontTestInterface, text1) == 8) ? 1 : -1];
	typedef char FontTestOriginalText2[(offsetof(FontTestInterface, text2) == 12) ? 1 : -1];
	typedef char FontTestOriginalText3[(offsetof(FontTestInterface, text3) == 16) ? 1 : -1];
	typedef char FontTestOriginalText4[(offsetof(FontTestInterface, text4) == 20) ? 1 : -1];
	typedef char FontTestOriginalEditor[(offsetof(FontTestInterface, editor) == 24) ? 1 : -1];
#endif
}
