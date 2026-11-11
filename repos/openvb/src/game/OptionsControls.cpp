#include "game/OptionsControls.hpp"
#include "game/OptionsServices.hpp"
#include "gui/Button.hpp"
#include "gui/Picture.hpp"
#include "gui/ScrollBar.hpp"
#include "audio/Sound.hpp"
#include <string>

OptionDial::OptionDial() : value(0), count(1), picture(NULL) {} //4431B0.
OptionDial::~OptionDial() {} //4431D0.
void OptionDial::BindPicture(::GUI::Picture* selected) { picture = selected; } //4431E0.
void OptionDial::SetCount(LONG selected) { count = selected; } //4431F0.
::GUI::Picture* OptionDial::GetPicture() const { return picture; } // Shared5C0160.

void OptionDial::Next() //443200.
{
	LONG next = LONG(DWORD(value) + 1);
	if (next >= count)
		next = 0;
	SetValue(next);
	OnChanged();
}

void OptionDial::OnChanged() //443220.
{
	JE::CSound* sound = new JE::CSound;
	if (sound)
	{
		sound->Load("Toggle.wav", 0);
		sound->Play(2, 0, 0);
	}
}

void OptionDial::SetValue(LONG selected) //443BE0.
{
	value = selected;
	if (picture)
	{
		switch (selected)
		{
		case 1: picture->LoadTexture(std::string("f3_options_dial_02.tga"), 0, 0, 0, -1, -1, 1); break;
		case 2: picture->LoadTexture(std::string("f3_options_dial_03.tga"), 0, 0, 0, -1, -1, 1); break;
		case 3: picture->LoadTexture(std::string("f3_options_dial_04.tga"), 0, 0, 0, -1, -1, 1); break;
		default: picture->LoadTexture(std::string("f3_options_dial_01.tga"), 0, 0, 0, -1, -1, 1); break;
		}
	}
}

OptionSwitch::OptionSwitch() { count = 2; } //4432A0.
OptionSwitch::~OptionSwitch() {} //4432C0.
void OptionSwitch::SetValue(LONG selected) //443D70.
{
	value = selected;
	if (picture)
	{
		if (selected == 1)
			picture->LoadTexture(std::string("f3_options_switch_02.tga"), 0, 0, 0, -1, -1, 1);
		else
			picture->LoadTexture(std::string("f3_options_switch_01.tga"), 0, 0, 0, -1, -1, 1);
	}
}

OptionCheckBox::OptionCheckBox() { count = 2; } //4432D0.
OptionCheckBox::~OptionCheckBox() {} //4432F0.
void OptionCheckBox::SetValue(LONG selected) //443E90.
{
	value = selected;
	if (picture)
	{
		if (selected == 1)
			picture->LoadTexture(std::string("f3_options_check_02.tga"), 0, 0, 0, -1, -1, 1);
		else
			picture->LoadTexture(std::string("f3_options_check_01.tga"), 0, 0, 0, -1, -1, 1);
	}
}

OptionScrollBar::OptionScrollBar() //443300.
{
	scrollbar = new ::GUI::ScrollBar;
	scrollbar->SetFlags(1);
	scrollbar->SetPageSize(1);
	scrollbar->SetMaximum(47);
	scrollbar->SetStep(1);
	scrollbar->SetPageStep(1);
}

OptionScrollBar::~OptionScrollBar() //4433A0.
{
	if (scrollbar)
	{
		delete scrollbar;
		scrollbar = NULL;
	}
}

void OptionScrollBar::SetMaximum(LONG value) //4433C0.
{
	if (scrollbar)
		scrollbar->SetMaximum(value);
}

void OptionScrollBar::BindThumb(::GUI::Window* value) //443FB0.
{
	if (scrollbar)
		scrollbar->BindThumb(value);
	if (value)
	{
		::GUI::Button* button = static_cast<::GUI::Button*>(value);
		button->SetSoundDown(std::string(""));
		button->SetSoundUp(std::string(""));
		button->SetSoundEnter(std::string(""));
	}
}

void OptionScrollBar::BindTrack(::GUI::Window* value) //4440A0.
{
	if (scrollbar)
		scrollbar->BindTrack(value);
	if (value)
	{
		::GUI::Button* button = static_cast<::GUI::Button*>(value);
		button->SetSoundDown(std::string(""));
		button->SetSoundUp(std::string(""));
		button->SetSoundEnter(std::string(""));
	}
}

