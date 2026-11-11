#include <winsock2.h>
#include "game/NameAgeSexInterface.hpp"
#include "game/CharacterGenerationInterface.hpp"
#include "game/CounterControl.hpp"
#include "game/GamePlayer.hpp"
#include "gui/Button.hpp"
#include "gui/Edit.hpp"
#include "gui/Event.hpp"
#include "gui/Manager.hpp"
#include "network/EngineClient.hpp"
#include "graphics/ClientCursor.hpp"
#include <cstring>

namespace F3
{
	NameAgeSexInterface::NameAgeSexInterface() : JE::EngineInterface("CharGen_NAS"), GameInterface("CharGen_NAS"), ageCounter(NULL)
	{
		LoadResource("CharGen_NAS.INT");
		ageCounter = new CounterControl;
		ageCounter->SetDigits(2);
		ageCounter->BindDigit(0, FindWindow("winAge1"));
		ageCounter->BindDigit(1, FindWindow("winAge10"));
		ageCounter->BindButtons(FindWindow("btnAge+"), FindWindow("btnAge-"));
		ageCounter->animationStep = 0.05f;
		SetHidden(1);
	}
	NameAgeSexInterface::~NameAgeSexInterface() { delete ageCounter; }
	void NameAgeSexInterface::Show(CharacterGenerationInterface* owner, GamePlayer* value,
		const std::string& text, LONG years, LONG gender)
	{
		parent = owner; character = value; name = text; age = years; sex = gender;
		::GUI::Edit* edit = static_cast< ::GUI::Edit*>(FindWindow("editName"));
		edit->SetText(name); edit->SetSelection(0, static_cast<LONG>(name.size()));
		ageCounter->Finish(); RefreshAge(); RefreshSex();
		::GUI::Window* root = FindWindow("winNAS");
		root->BringToFront(); SetHidden(0); ::GUI::g_Manager.PushModal(this);
	}
	void NameAgeSexInterface::GetValues(std::string& text, LONG& years, LONG& gender) //426BF0.
	{
		text = name;
		years = age;
		gender = sex;
	}
	void NameAgeSexInterface::RefreshSex()
	{
		::GUI::Button* button = static_cast< ::GUI::Button*>(FindWindow("btnMale"));
		button->SetChecked(sex == 0);
		button = static_cast< ::GUI::Button*>(FindWindow("btnFemale"));
		button->SetChecked(sex == 1);
	}
	void NameAgeSexInterface::Close()
	{
		ageCounter->Finish(); SetHidden(1); ::GUI::g_Manager.RemoveModal(this);
	}
	void NameAgeSexInterface::BeforeUpdate() { ageCounter->Update(); }
	int NameAgeSexInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (ageCounter && ageCounter->Process(window, event))
		{
			if (event->type == 4 || event->type == 7)
			{
				if (window->type == 1 && (window == ageCounter->decrementButton || window == ageCounter->incrementButton))
				{ age = ageCounter->GetValue(); parent->SetAgeLabel(age); }
			}
			else if (event->type == 0)
			{
				if (window->type == 1 && (window == ageCounter->incrementButton || window == ageCounter->decrementButton))
				{ age = ageCounter->GetValue(); parent->SetAgeLabel(age); }
			}
			return JE::EngineInterface::OnEvent(window, event);
		}
		if (event->type == 6)
		{
			if (window->type == 1)
			{
				if (!_stricmp(window->GetName(), "btnDone"))
				{ parent->ApplyNameAgeSex(); Close(); }
				else
				{
					if (!_stricmp(window->GetName(), "btnMale"))
					{ sex = 0; parent->SetSexLabel(sex); }
					else if (!_stricmp(window->GetName(), "btnFemale"))
					{ sex = 1; parent->SetSexLabel(sex); }
					else return JE::EngineInterface::OnEvent(window, event);
					RefreshSex(); parent->ApplyNameAgeSex(); parent->ResetAppearance();
				}
			}
			return JE::EngineInterface::OnEvent(window, event);
		}
		if ((event->type == 19 || event->type == 17) && window->type == 4 &&
			!_stricmp(window->GetName(), "editName"))
		{
			bool finish = false;
			switch (event->x)
			{
			case 1:
				parent->ApplyNameAgeSex(); Close(); return 1;
			case 28: case 156: case 284:
				finish = true;
			}
			{
				::GUI::Edit* edit = static_cast< ::GUI::Edit*>(window);
				std::string before;
				edit->GetText(before);
				edit->OnEvent(event);
				edit->GetText(name);
				if (name != before) parent->SetNameLabel(name);
			}
			if (finish) { parent->ApplyNameAgeSex(); Close(); }
			return 1;
		}
		return JE::EngineInterface::OnEvent(window, event);
	}

}
