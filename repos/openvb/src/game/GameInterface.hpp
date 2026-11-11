#pragma once

#include "gui/EngineInterface.hpp"

namespace F3
{
	//6F34FC: shared default-empty text object. Original static lifetime remains unmeasured.
	extern std::string g_EmptyInterfaceText;
	//4395A0/6561E4: EngineInterface is the actual virtual base; its own
	//GUI::Interface base remains ordinary. Most-derived classes construct it.
	class GameInterface : virtual public JE::EngineInterface
	{
	public:
		explicit GameInterface(const std::string& name); //4395A0; actual EngineInterface base.
		virtual ~GameInterface(); //4395F0, then qualified base lifetime.
		virtual BYTE OnCursor(::GUI::Window* window, LONG x, LONG y);
	};
}
