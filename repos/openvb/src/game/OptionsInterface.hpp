#pragma once

#include "game/GameInterface.hpp"

namespace GUI { class Label; }
namespace F3
{
	class OptionsInterface : public GameInterface
	{
	public:
		explicit OptionsInterface(const std::string& name); //4436D0.
		virtual ~OptionsInterface(); //4433D0.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //443410/443570.
		void Reset(); //442BF0.
		LONG CurrentPage() const; //442D10.
		BYTE SwitchPage(LONG page); //442DC0.
		BYTE Open(LONG page); //4433E0.
		BYTE CloseChildren(); //443400.
		void PlayTabSound(); //443130, ordinary unused-receiver sound request.

		::GUI::Window* rootWindow;
		::GUI::Label* labels[5][2]; //Game, HotKeys, Load, Multiplayer, Video pairs.
	};
}
