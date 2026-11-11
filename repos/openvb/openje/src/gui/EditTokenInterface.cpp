#include <winsock2.h>
#include "EditTokenInterface.hpp"
#include "gui/Event.hpp"
#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "startup/EngineStringSystem.hpp"
#include <cstring>

namespace JE
{
	EditTokenInterface::EditTokenInterface(const std::string& name) : EngineInterface(name)
	{
		std::string key("EditToken.INT");
		LoadResource(key);
		key.assign("Main");
		main = FindWindow(key);
		key.assign("Up");
		up = main->FindWindow(key);
		key.assign("Down");
		down = main->FindWindow(key);
	}


	void EditTokenInterface::Confirm()
	{
		up->SetHidden(1);
		down->SetHidden(0);
		::GUI::Window* edit;
		{
			const std::string key("Edit");
			edit = main->FindWindow(key);
		}
		if (edit)
		{
			std::string text;
			static_cast< ::GUI::Label*>(edit)->GetText(text);
			F3::g_EngineStringSystem.SetTokenText(tokenName.c_str(), text.c_str());
			main->SetHidden(1);
		}
		::GUI::g_Manager.RemoveModal(this);
	}

	int EditTokenInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		switch (event->type)
		{
		case 17:
			if (event->x == 28 || event->x == 284) Confirm();
			break;
		case 6:
			if (!_stricmp(window->GetName(), "Up")) Confirm();
			break;
		}
		return EngineInterface::OnEvent(window, event);
	}
}
