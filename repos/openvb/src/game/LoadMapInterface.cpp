#include <winsock2.h>
#include "game/LoadMapInterface.hpp"
#include "game/GameInterfaces.hpp"
#include "game/MainMenuInterface.hpp"
#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "gui/Event.hpp"
#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "gui/Window.hpp"
#include "audio/Sound.hpp"
#include <cstring>
#include <stdexcept>

namespace F3
{
	namespace
	{
		//The pointer range6F3020..6F3058 contains fourteen entries. The ctor's
		//initial6 is the row Y coordinate, not the number of names.
		const char* mapNames[] =
		{
			"DemoBeaconTorInterior", "DemoWhiteFordNWQuad",
			"", "", "", "", "", "", "", "", "", "", "", ""
		};
	}

	LoadMapInterface::LoadMapInterface(const std::string& name) : ::GUI::Interface(name)
	{
		clearOnFirstCharacter = 1;
		LoadResource("LoadMap.INT"); //Original ignores the reader result.
		::GUI::Window* namesWindow = FindWindow("MapNamesWin");
		if (namesWindow)
		{
			LONG rowY = 6;
			for (const char** entry=mapNames; entry<mapNames+sizeof(mapNames)/sizeof(mapNames[0]); ++entry)
			{
				::GUI::Label* label = new ::GUI::Label(3);
				namesWindow->AddChild(label); //Publish before configuring the live child.
				label->Resize(260, 16);
				label->ApplySkin("mainmenu.ini");
				label->SetFont("Arial", 0, 10, 1);
				label->SetTextColor(0xFF808080);
				label->SetText(*entry);
				label->SetClientPosition(8, rowY);
				label->SetValue3B(0);
				rowY += 16;
			}
		}
		mapNameEdit = static_cast<::GUI::Label*>(FindWindow("MapNameEdit"));
		if (mapNameEdit)
			mapNameEdit->SetText(mapNames[0]);
		clickSound = new JE::CSound;
		clickSound->Load("btnclick01.wav", 0); //Publish the pointer before virtual Load.
	}

	LoadMapInterface::~LoadMapInterface()
	{
		if (clickSound)
			delete clickSound;
		//Original Interface cleanup retains its selective child-deletion policy.
	}

	int LoadMapInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (event->type == 4)
		{
			if (window->type == 3)
			{
				//Original virtual+13C/4A8A90 only copies the actual text string.
				std::string text;
				static_cast<::GUI::Label*>(window)->GetText(text);
				if (text != "")
				{
					mapNameEdit->SetText(text);
					if (clickSound)
						clickSound->Play(0, 0, 0);
				}
			}
		}
		else if (event->type == 6)
		{
			if (window->type == 1)
			{
				if (clickSound) clickSound->Play(0, 0, 0);
			}
			if (!_stricmp(window->GetName(), "LoadBtn"))
			{
				//Same original4A8A90 value copy for the actual serialized Edit.
				std::string text;
				mapNameEdit->GetText(text);
				if (text != "")
				{
					//The original constructs and destroys this temporary. It sends
					//no command/request and does not load the named map.
					const std::string map = text + ".map";
				}
				SetHidden(1);
				::GUI::g_Manager.RemoveModal(this);
				GameClient* client = static_cast<GameClient*>(g_pNetworkInstance->client);
				GameInterfaces* interfaces = client->Interfaces();
				MainMenuInterface* menu = interfaces->mainMenu;
				if (menu)
				{
					menu->SetHidden(1);
					::GUI::g_Manager.RemoveModal(menu);
				}
			}
			else if (!_stricmp(window->GetName(), "CancelBtn"))
			{
				SetHidden(1);
				::GUI::g_Manager.RemoveModal(this);
			}
		}
		else if (event->type == 19 && clearOnFirstCharacter)
		{
			mapNameEdit->SetText("");
			clearOnFirstCharacter = 0;
		}
		return ::GUI::Interface::OnEvent(window, event);
	}
}
