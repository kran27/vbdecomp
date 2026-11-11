#pragma once

#include "game/GameInterface.hpp"
#include "game/OptionsControls.hpp"

class COptionsGameInterface : public F3::GameInterface
{
public:
	explicit COptionsGameInterface(const std::string& name); //444530.
	virtual ~COptionsGameInterface(); //444190, then normal base lifetime.
	virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //444260.

	::GUI::Window* rootWindow; //Original+04; published from FindWindow.
	::OptionDial dials[5]; //Original+08, actual16-byte elements.
	::OptionSwitch switches[6]; //+58.
	::OptionCheckBox checkBoxes[1]; //+B8.
	::OptionScrollBar sliders[8]; //+C8; owns only the8 ScrollBars.
};

namespace F3
{
	typedef ::COptionsGameInterface GameOptionsInterface;
}

