#pragma once

#include "game/GameInterface.hpp"

namespace F3
{
	class LoadingBarInterface : public GameInterface
	{
	public:
		explicit LoadingBarInterface(const std::string& name); //4407C0.
		virtual ~LoadingBarInterface(); //440730 ordinary phase.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //440780 ->440760.
		::GUI::Window* loadingWindow; //Original+04, borrowed LoadingWin.
	};
}
