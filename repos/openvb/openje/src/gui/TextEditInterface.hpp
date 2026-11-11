#pragma once

#include "gui/EngineInterface.hpp"

namespace GUI { class Window; class Edit; }

namespace JE
{
	class TextEditInterface : public virtual EngineInterface
	{
	public:
		explicit TextEditInterface(const std::string& name); //5B7C10, allocation4Ch.
		virtual ~TextEditInterface(); //5B7780.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //5B8A00/5B8880.
		void Apply(); //5B7820, actual current Client98 FontTest owner.
		void SetMode(DWORD value); //5B7DD0, reads current values before publishing mode.
		void Clear(); //5B8050, actual empty-string SetText.

		::GUI::Window* main; //04, borrowed AddNoteWin.
		::GUI::Edit* edit; //08, borrowed NoteEditText.
		BYTE clearOnFirstKey; //0C; padding0D..0F unwritten.
		DWORD mode; //10; vtordisp14, real EngineInterface18.
	};

#if defined(_MSC_VER) && _MSC_VER == 1300
	typedef char TextEditOriginalExtent[(sizeof(TextEditInterface) == 0x4C) ? 1 : -1];
	typedef char TextEditOriginalRoot[(offsetof(TextEditInterface, main) == 4) ? 1 : -1];
	typedef char TextEditOriginalEdit[(offsetof(TextEditInterface, edit) == 8) ? 1 : -1];
	typedef char TextEditOriginalClear[(offsetof(TextEditInterface, clearOnFirstKey) == 12) ? 1 : -1];
	typedef char TextEditOriginalMode[(offsetof(TextEditInterface, mode) == 16) ? 1 : -1];
#endif
}
