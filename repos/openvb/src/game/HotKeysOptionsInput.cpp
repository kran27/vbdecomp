#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/HotKeysOptionsInterface.hpp"
#include "game/GameHotKeys.hpp"
#include "game/GameInterfaces.hpp"
#include "game/OptionsInterface.hpp"
#include "graphics/ClientInput.hpp"
#include "gui/Event.hpp"
#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "gui/Picture.hpp"
#include <cstring>

namespace F3
{
	void HotKeysOptionsInterface::ReplaceCommandBinding(LONG scan, DWORD subtype, DWORD modifiers,
		const char* command) //4452B0.
	{
		JE::ClientHotKeys& keys = *static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x80;
		DWORD previousScan = DWORD(-1), previousSubtype, previousModifiers;
		while (keys.FindBinding(previousScan, previousSubtype, previousModifiers,
			command) == 1)
		{
			keys.Unbind(previousScan, previousSubtype, previousModifiers);
		}
		if (scan != -1)
			keys.Bind(scan, subtype, modifiers, command);
	}

	void HotKeysOptionsInterface::FormatKey(std::string& output, LONG scan, DWORD subtype,
		DWORD modifiers) //445D20, RET16; subtype is not read.
	{
		output = "";
		if (scan != -1)
		{
			//Original tests only LOWBYTE(modifiers), Shift before Control.
			if ((static_cast<BYTE>(modifiers) & 2) != 0)
				output = "Shift-";
			else if ((static_cast<BYTE>(modifiers) & 1) != 0)
				output = "Ctrl-";
			JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
			const char* keyName = client->child0x78->KeyName(static_cast<DWORD>(scan));
			output = output + keyName;
		}
	}

	BYTE HotKeysOptionsInterface::IsEditing() //445350.
	{
		for (std::list<HotKeyRow*>::iterator it = rows.begin(); it != rows.end(); ++it)
			if (*it && (*it)->editing)
				return 1;
		return 0;
	}

	void HotKeysOptionsInterface::Commit() //445380.
	{
		for (std::list<HotKeyRow*>::iterator it = rows.begin(); it != rows.end(); ++it)
		{
			HotKeyRow* row = *it;
			if (!row)
				continue;
			if (_stricmp(row->command.c_str(), "CameraZoomIn_On") == 0)
			{
				ReplaceCommandBinding(row->scan, 0, row->modifiers, "CameraZoomIn_On");
				ReplaceCommandBinding(row->scan, 1, row->modifiers, "CameraZoomIn_Off");
			}
			else if (_stricmp(row->command.c_str(), "CameraZoomOut_On") == 0)
			{
				ReplaceCommandBinding(row->scan, 0, row->modifiers, "CameraZoomOut_On");
				ReplaceCommandBinding(row->scan, 1, row->modifiers, "CameraZoomOut_Off");
			}
			else if (_stricmp(row->command.c_str(), "CameraMouseRotate_On") == 0)
			{
				ReplaceCommandBinding(row->scan, 0, row->modifiers, "CameraMouseRotate_On");
				ReplaceCommandBinding(row->scan, 1, row->modifiers, "CameraMouseRotate_Off");
			}
			else if (_stricmp(row->command.c_str(), "CameraScrollUp_On") == 0)
			{
				ReplaceCommandBinding(row->scan, 0, row->modifiers, "CameraScrollUp_On");
				ReplaceCommandBinding(row->scan, 1, row->modifiers, "CameraScrollUp_Off");
			}
			else if (_stricmp(row->command.c_str(), "CameraScrollDown_On") == 0)
			{
				ReplaceCommandBinding(row->scan, 0, row->modifiers, "CameraScrollDown_On");
				ReplaceCommandBinding(row->scan, 1, row->modifiers, "CameraScrollDown_Off");
			}
			else if (_stricmp(row->command.c_str(), "CameraScrollLeft_On") == 0)
			{
				ReplaceCommandBinding(row->scan, 0, row->modifiers, "CameraScrollLeft_On");
				ReplaceCommandBinding(row->scan, 1, row->modifiers, "CameraScrollLeft_Off");
			}
			else if (_stricmp(row->command.c_str(), "CameraScrollRight_On") == 0)
			{
				ReplaceCommandBinding(row->scan, 0, row->modifiers, "CameraScrollRight_On");
				ReplaceCommandBinding(row->scan, 1, row->modifiers, "CameraScrollRight_Off");
			}
			else
				ReplaceCommandBinding(row->scan, 0, row->modifiers, row->command.c_str());
		}
	}

	void HotKeysOptionsInterface::RestoreDefaults() //4455D0.
	{
		//Capture the actual nullable GameHotKeys RTTI result before changing rows.
		JE::ClientHotKeys* keys = static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x80;
		GameHotKeys* gameKeys = dynamic_cast<GameHotKeys*>(keys);
		for (std::list<HotKeyRow*>::iterator it = rows.begin(); it != rows.end(); ++it)
			if (*it)
				(*it)->scan = -1;
		Commit();
		if (gameKeys)
			gameKeys->LoadGameDefaults(); //468AE0; no unconditional game-owner claim.
	}

	void HotKeysOptionsInterface::BeginEdit(HotKeyRow* row) //445970.
	{
		::GUI::g_Manager.PushModal(this);
		row->cell->LoadTexture("f3_options_hotkey_edit.tga", 0, 0, 0, -1, -1, 1);
		row->editing = 1;
	}

	void HotKeysOptionsInterface::EndEdit() //445A40.
	{
		for (std::list<HotKeyRow*>::iterator it = rows.begin(); it != rows.end(); ++it)
		{
			HotKeyRow* row = *it;
			if (row && row->editing)
			{
				row->cell->LoadTexture("f3_options_hotkey_cell.tga", 0, 0, 0, -1, -1, 1);
				row->editing = 0;
			}
		}
		::GUI::g_Manager.RemoveModal(this);
	}

	void HotKeysOptionsInterface::Remap(LONG scan, DWORD modifiers) //445E20.
	{
		for (std::list<HotKeyRow*>::iterator it = rows.begin(); it != rows.end(); ++it)
		{
			HotKeyRow* editing = *it;
			if (editing && editing->editing)
			{
				std::string display;
				FormatKey(display, scan, 0, modifiers);
				for (std::list<HotKeyRow*>::iterator it = rows.begin(); it != rows.end(); ++it)
				{
					//Native445EE0 owns this per-row string and its unwind state.
					std::string rowText;
					HotKeyRow* row = *it;
					if (row && _stricmp(row->display.c_str(), display.c_str()) == 0)
					{
						row->keyLabel->SetText("");
						row->display = "";
						row->scan = -1;
						row->modifiers = 0;
					}
				}
				editing->keyLabel->SetText(display);
				editing->display = display;
				editing->scan = scan;
				editing->modifiers = modifiers;
				EndEdit();
				return;
			}
		}
	}

	void HotKeysOptionsInterface::ReloadBindings() //446080.
	{
		DWORD subtype, modifiers; //Original output slots persist across row iterations.
		for (std::list<HotKeyRow*>::iterator it = rows.begin(); it != rows.end(); ++it)
		{
			std::string display;
			HotKeyRow* row = *it;
			if (row)
			{
				JE::ClientHotKeys& keys = *static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x80;
				DWORD scan = DWORD(-1); //Only scan resets; missing bindings retain other outputs.
				keys.FindBinding(
					scan, subtype, modifiers, row->command.c_str());
				FormatKey(display, static_cast<LONG>(scan), subtype, modifiers);
				row->keyLabel->SetText(display);
				row->display = display;
				row->scan = static_cast<LONG>(scan);
				row->modifiers = modifiers;
			}
		}
	}

	void HotKeysOptionsInterface::PrepareForOpen() //4461D0.
	{
		EndEdit();
		ReloadBindings();
	}

	int HotKeysOptionsInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (GetHidden()) //4465E0;4468B0 is the virtual-base adjustor.
			return ::GUI::Interface::OnEvent(window, event);
		if (event->type == 6)
		{
			if (_stricmp(window->GetName(), "btnDefault") == 0)
			{
				EndEdit();
				RestoreDefaults();
				ReloadBindings();
			}
			else if (_stricmp(window->GetName(), "btnDone") == 0)
			{
				OptionsInterface* options = static_cast<GameInterfaces*>(static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xA0)->options;
				EndEdit();
				Commit();
				options->CloseChildren();
				options->SetHidden(1);
			}
			else if (_stricmp(window->GetName(), "btnCancel") == 0)
			{
				OptionsInterface* options = static_cast<GameInterfaces*>(static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xA0)->options;
				EndEdit();
				ReloadBindings();
				options->CloseChildren();
				options->SetHidden(1);
			}
		}
		else if (event->type == 4)
		{
			EndEdit();
			for (std::list<HotKeyRow*>::iterator it = rows.begin(); it != rows.end(); ++it)
			{
				HotKeyRow* row = *it;
				if (row && window == row->cell)
				{
					BeginEdit(row);
					break;
				}
			}
		}
		else if (event->type == 17 && window == background)
		{
			switch (event->x)
			{
			case 1:
				if (IsEditing())
					EndEdit();
				else
				{
					OptionsInterface* options = static_cast<GameInterfaces*>(static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xA0)->options;
					EndEdit();
					options->CloseChildren();
					options->SetHidden(1);
					return 1;
				}
				break;
			case 29:
			case 42:
			case 54:
			case 157:
				break;
			default:
				if (IsEditing())
					Remap(event->x, event->modifiers);
				break;
			}
		}
		return JE::EngineInterface::OnEvent(window, event);
	}
}
