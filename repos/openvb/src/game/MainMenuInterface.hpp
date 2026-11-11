#pragma once

#include "game/GameInterface.hpp"

namespace F3
{
	class MainMenuInterface : public GameInterface
	{
	public:
		explicit MainMenuInterface(const std::string& name); //440990.
		virtual ~MainMenuInterface(); //440910 ordinary phase.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //440CE0/440FB0.
		void RefreshSingleMulti(); //440AF0, actual three optional Labels.
		void PrepareForOpen(); //404B70, ordinary empty hook before initialization visibility.

		BYTE singleMulti; //Original+04, raw BYTE, constructor0.
		::GUI::Window* mainWindow; //Original+08, borrowed MainmenuWin.
	};
}
