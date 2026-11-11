#pragma once

#include "game/GameInterface.hpp"

namespace F3
{
	class OptionsMultiplayerInterface : public GameInterface
	{
	public:
		explicit OptionsMultiplayerInterface(const std::string& name); //4491F0.
		virtual ~OptionsMultiplayerInterface(); //449100.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //449130.

		::GUI::Window* rootWindow; //Original+04, borrowed serialized root.
	};
}
