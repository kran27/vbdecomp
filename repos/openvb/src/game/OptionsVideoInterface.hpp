#pragma once

#include "game/GameInterface.hpp"
#include "game/OptionsControls.hpp"

namespace F3
{
	class OptionsVideoInterface : public GameInterface
	{
	public:
		explicit OptionsVideoInterface(const std::string& name); //4496A0.
		virtual ~OptionsVideoInterface(); //449340.
		void PrepareForOpen(); //Shared404B70, unused receiver/no arguments.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //449400.

		::GUI::Window* rootWindow; //Original+04, borrowed serialized root.
		OptionDial dials[2]; //+08, original16-byte elements.
		OptionSwitch switches[6]; //+28.
		OptionScrollBar sliders[8]; //+88, original8-byte elements.
	};
}
