#pragma once

#include "gui/EngineInterface.hpp"

namespace GUI { class Label; }

namespace JE
{
	class CSound;
	class DebugToolsInterface : public virtual EngineInterface
	{
	public:
		explicit DebugToolsInterface(const std::string& name); //5B6400, virtual base+3C.
		virtual ~DebugToolsInterface(); //5B5CE0, then the real virtual base lifetime.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //5B7750/5B6BC0.
		void EnableControls(BYTE enabled); //5B6270.
		void SetPosition(LONG x, LONG y); //5B5C50.
		void Drag(LONG x, LONG y); //5B5CB0.
		int EditCharacter(::GUI::Window*, ::GUI::Event* event); //5B5D60.
		int EditKey(::GUI::Window* window, ::GUI::Event* event); //5B5EB0.

		LONG x, y, width, height; //Original04..10.
		LONG oldX, oldY, grabX, grabY, dragState; //14..24.
		::GUI::Window* main; //28, DebugToolsWin.
		CSound* sound; //2C, owns the ordinary nonvirtual CSound.
		::GUI::Label* entity; //30, authored CurEntityLbl (an Edit).
		BYTE editing; //34;35..37 padding unwritten, vtordisp38, EngineInterface3C.
	};
}
