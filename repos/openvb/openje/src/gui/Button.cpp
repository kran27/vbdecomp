#include "Button.hpp"
#include "Event.hpp"
#include "Manager.hpp"
#include "ResourceIO.hpp"
#include "resources/ResourceSystem.hpp"
#include "FileString.hpp"
#include <cstring>
#include <stdexcept>

namespace GUI
{
	namespace
	{
		// Original globals6FAE5C/78/94 are constructed by641490/C0/F0.
		const std::string DefaultDown("f3_button_down_01.wav");
		const std::string DefaultUp("f3_button_up_01.wav");
		const std::string DefaultEnter("f3_button_up_01.wav");
		const char* const Profiles[8] =
		{
			"button.up", "button.selected", "button.down", "button.disabled",
			"button.checked.up", "button.checked.selected", "button.checked.down",
			"button.checked.disabled"
		};
	}

	void Button::SetSoundDown(std::string value) //4435B0.
	{
		soundDown = value;
	}
	void Button::SetSoundUp(std::string value) //443610.
	{
		soundUp = value;
	}
	void Button::SetSoundEnter(std::string value) //443670.
	{
		soundEnter = value;
	}

	Button::Button(DWORD type) : Window(type), state(0), checked(0), keyboardEnabled(0)
	{
		soundDown = DefaultDown;
		soundUp = DefaultUp;
		soundEnter = DefaultEnter;
	}

	Button::~Button()
	{
	}

	BYTE Button::ReadResource(JE::Resources::ResourceFileHandle* file, int version)
	{
		if (!JE::Resources::g_pResourceSystem->Read(&state, 4, file)) return 0;
		if (version >= 4)
		{
			BYTE value;
			if (!JE::Resources::g_pResourceSystem->Read(&value, 1, file)) return 0;
			keyboardEnabled = value != 0;
			if (!ReadResourceString(soundDown, file)
				|| !ReadResourceString(soundUp, file)
				|| !ReadResourceString(soundEnter, file)) return 0;
		}
		return Window::ReadResource(file, version) != 0;
	}

	BYTE Button::ReadFile(FILE* file, int version) //4A6A10.
	{
		if (std::fread(&state, 4, 1, file) != 1) return 0;
		if (version >= 4)
		{
			BYTE value;
			if (std::fread(&value, 1, 1, file) != 1) return 0;
			keyboardEnabled = value != 0;
			if (!ReadFileString(soundDown, file)
				|| !ReadFileString(soundUp, file)
				|| !ReadFileString(soundEnter, file)) return 0;
		}
		return Window::ReadFile(file, version) != 0;
	}

	BYTE Button::WriteFile(FILE* file) //4A6AC0.
	{
		if (std::fwrite(&state, 4, 1, file) != 1) return 0;
		BYTE value = keyboardEnabled != 0;
		if (std::fwrite(&value, 1, 1, file) != 1
			|| !WriteFileString(soundDown, file)
			|| !WriteFileString(soundUp, file)
			|| !WriteFileString(soundEnter, file)) return 0;
		return Window::WriteFile(file) != 0;
	}

	DWORD Button::GetState()
	{
		return state;
	}

	BYTE Button::SetKeyboardEnabled(BYTE value)
	{
		keyboardEnabled = value;
		return value;
	}
	bool Button::IsEnabled()
	{
		if (state != 3 && state != 7) return true;
		return false;
	}
	void Button::SetEnabled(BYTE enabled) //4A6CE0.
	{
		switch (state)
		{
		case 0: case 1: case 2:
			if (!enabled) state = 3;
			break;
		case 3:
			if (enabled) state = 0;
			break;
		case 4: case 5: case 6:
			if (!enabled) state = 7;
			break;
		case 7:
			if (enabled) state = 4;
			break;
		}
		ApplySkin(skin);
	}

	BOOL Button::OnEvent(Event* event)
	{
		if (!IsEnabled()) return FALSE;
		bool apply = true;
		switch (event->type)
		{
		case 1:
			if (!(event->buttons & 1)) state = 4 * (checked != 0) + 1;
			else if (g_Manager.pressedLeft == this) state = 4 * (checked != 0) + 2;
			break;
		case 3: state = 4 * (checked != 0); break;
		case 4:
		case 7: state = 4 * (checked != 0) + 2; break;
		case 5: state = 4 * (checked != 0) + 1; break;
		case 12:
		case 15:
			if (keyboardEnabled) state = 4 * (checked != 0) + 2;
			else apply = false;
			break;
		case 13:
			if (keyboardEnabled) state = 4 * (checked != 0) + 1;
			else apply = false;
			break;
		default: apply = false;
		}
		if (apply) ApplySkin(skin);
		return Window::OnEvent(event);
	}

	void Button::SetChecked(BYTE value)
	{
		if (checked == value) return;
		checked = value;
		if (value)
		{
			switch (state)
			{
			case 0: state = 4; break;
			case 1: state = 5; break;
			case 2: state = 6; break;
			case 3: state = 7; break;
			}
		}
		else
		{
			switch (state)
			{
			case 4: state = 0; break;
			case 5: state = 1; break;
			case 6: state = 2; break;
			case 7: state = 3; break;
			}
		}
		ApplySkin(skin);
	}

	void Button::ApplySkin(const std::string& skinName)
	{
		// The original indexes this table without a bounds check.
		{
			const std::string profile(Profiles[state]);
			Window::ApplyProfile(skinName, profile);
		}
		std::list<Window*>* list = GetChildren();
		Window* down = NULL;
		Window* up = NULL;
		std::list<Window*> selected;
		for (std::list<Window*>::iterator current = list->begin(); current != list->end(); ++current)
		{
			Window* child = *current;
			if (!_strnicmp(child->GetName(), "BtnUp", 5))
			{
				if (state == 2 || state == 1 || state == 0 || state == 3) up = child;
				child->SetValue3B(1); child->SetHidden(1);
				if (state == 0) selected.push_back(child);
			}
			else if (!_strnicmp(child->GetName(), "BtnSelected", 11))
			{
				child->SetValue3B(1); child->SetHidden(1);
				if (state == 1) selected.push_back(child);
			}
			else if (!_strnicmp(child->GetName(), "BtnDown", 7))
			{
				if (state == 6 || state == 5 || state == 4 || state == 7) down = child;
				child->SetValue3B(1); child->SetHidden(1);
				if (state == 2) selected.push_back(child);
			}
			else if (!_strnicmp(child->GetName(), "BtnDisabled", 11))
			{
				child->SetValue3B(1); child->SetHidden(1);
				if (state == 3) selected.push_back(child);
			}
			else if (!_strnicmp(child->GetName(), "BtnCheckedUp", 5))
			{
				child->SetValue3B(1); child->SetHidden(1);
				if (state == 4) selected.push_back(child);
			}
			else if (!_strnicmp(child->GetName(), "BtnCheckedSelected", 11))
			{
				child->SetValue3B(1); child->SetHidden(1);
				if (state == 5) selected.push_back(child);
			}
			else if (!_strnicmp(child->GetName(), "BtnCheckedDown", 7))
			{
				child->SetValue3B(1); child->SetHidden(1);
				if (state == 6) selected.push_back(child);
			}
			else if (!_strnicmp(child->GetName(), "BtnCheckedDisabled", 11))
			{
				child->SetValue3B(1); child->SetHidden(1);
				if (state == 7) selected.push_back(child);
			}
			if (!down) down = child;
			if (!up) up = child;
		}
		if (!selected.empty())
		{
			for (std::list<Window*>::iterator current = selected.begin(); current != selected.end(); ++current)
				(*current)->SetHidden(0);
		}
		else if (state == 2 || state == 1 || state == 0 || state == 3)
		{
			if (up) up->SetHidden(0);
			else if (down) down->SetHidden(0);
		}
		else if (state == 6 || state == 5 || state == 4 || state == 7)
		{
			if (down) down->SetHidden(0);
			else if (up) up->SetHidden(0);
		}
	}
}
