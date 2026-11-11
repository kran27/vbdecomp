#include <winsock2.h>
#include "gui/DebugToolsInterface.hpp"
#include "gui/DebugWindow.hpp"
#include "gui/Button.hpp"
#include "gui/Label.hpp"
#include "gui/Event.hpp"
#include "gui/Manager.hpp"
#include "audio/Sound.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineServer.hpp"
#include "network/CommandProcessor.hpp"
#include "startup/NetworkComponent.hpp"
#include "startup/GameScriptingComponent.hpp"
#include "entity/Entity.hpp"
#include "scripting/ScriptSmall.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

namespace
{
	JE::Client* DebugClient() { return static_cast<JE::Client*>(F3::g_pNetworkInstance->client); }
	JE::Server* DebugServer() { return static_cast<JE::Server*>(F3::g_pNetworkInstance->server); }
	void Write(F3::ScriptingServices::DebugWindow* window, const char* text, int r, int g, int b)
	{
		F3::ScriptingServices::WriteToDebugWindow(window, std::string(text), r, g, b);
	}
	::GUI::Label* ButtonLabel(::GUI::Window* button)
	{
		return button ? static_cast< ::GUI::Label*>(button->FindWindow(std::string("Label"))) : NULL;
	}
}

namespace JE
{
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char DebugToolsSize[(sizeof(DebugToolsInterface) == 0x70) ? 1 : -1];
	typedef char DebugToolsX[(offsetof(DebugToolsInterface, x) == 4) ? 1 : -1];
	typedef char DebugToolsWidth[(offsetof(DebugToolsInterface, width) == 0x0C) ? 1 : -1];
	typedef char DebugToolsMain[(offsetof(DebugToolsInterface, main) == 0x28) ? 1 : -1];
	typedef char DebugToolsSound[(offsetof(DebugToolsInterface, sound) == 0x2C) ? 1 : -1];
	typedef char DebugToolsEntity[(offsetof(DebugToolsInterface, entity) == 0x30) ? 1 : -1];
	typedef char DebugToolsEditing[(offsetof(DebugToolsInterface, editing) == 0x34) ? 1 : -1];
#endif
	DebugToolsInterface::DebugToolsInterface(const std::string& name) : EngineInterface(name)
	{
		LoadResource(std::string("DebugTools.int"));
		main = FindWindow(std::string("DebugToolsWin"));
		entity = static_cast< ::GUI::Label*>(FindWindow(std::string("CurEntityLbl")));
		x = 0;
		y = 0;
		width = 0;
		height = 0;
		oldX = 0;
		oldY = 0;
		grabX = 0;
		grabY = 0;
		dragState = 0;
		editing = 0;
		if (main && entity)
		{
			LONG nextX, nextY, nextWidth, nextHeight;
			main->GetAbsolutePosition(nextX, nextY);
			main->GetSize(nextWidth, nextHeight);
			x = nextX;
			y = nextY;
			width = nextWidth;
			height = nextHeight;
			entity->SetText(std::string("None"));
		}
		else LogDebugString("Unable to load DebugTools.INT. DebugTools interface not properly initialized.");
		sound = new CSound;
		sound->Load("btnclick01.wav", 0); //Required even if allocation returned NULL.
		::GUI::Label* label = NULL;
		::GUI::Button* oneRound = static_cast< ::GUI::Button*>(FindWindow(std::string("OneRoundBtn")));
		if (oneRound) oneRound->SetEnabled(0);
		EnableControls(0); //Resume/Step/Abort are originally unchecked required controls.
		::GUI::Window* button = FindWindow(std::string("ToggleVarsBtn"));
		if (button)
		{
			label = static_cast< ::GUI::Label*>(button->FindWindow(std::string("Label")));
			if (label) label->SetText(std::string("Display Local Vars"));
		}
		button = FindWindow(std::string("ToggleBPsBtn"));
		if (button)
		{
			label = static_cast< ::GUI::Label*>(button->FindWindow(std::string("Label")));
			if (label) label->SetText(std::string("Disable Breakpoints"));
		}
		button = FindWindow(std::string("ToggleScripts"));
		if (button) label = static_cast< ::GUI::Label*>(button->FindWindow(std::string("Label")));
		// The original reuses label if ToggleScripts itself is absent.
		if (label) label->SetText(std::string("Disable Scripts"));
	}

	DebugToolsInterface::~DebugToolsInterface()
	{
		if (sound) { delete sound; sound = NULL; }
	}

	void DebugToolsInterface::EnableControls(BYTE enabled)
	{
		::GUI::Button* button = static_cast< ::GUI::Button*>(FindWindow(std::string("ResumeBtn")));
		button->SetEnabled(enabled);
		button = static_cast< ::GUI::Button*>(FindWindow(std::string("StepBtn")));
		button->SetEnabled(enabled);
		button = static_cast< ::GUI::Button*>(FindWindow(std::string("AbortBtn")));
		button->SetEnabled(enabled);
	}

	void DebugToolsInterface::SetPosition(LONG nextX, LONG nextY)
	{
		if (nextX < 0) nextX = 0;
		if (nextY < 0) nextY = 0;
		if (width + nextX > 1024) nextX = 1024 - width;
		if (height + nextY > 768) nextY = 768 - height;
		if (nextX != x || nextY != y)
		{
			x = nextX;
			y = nextY;
			main->SetAbsolutePosition(nextX, nextY);
		}
	}

	void DebugToolsInterface::Drag(LONG nextX, LONG nextY)
	{
		switch (dragState)
		{
		case 1: SetPosition(nextX - grabX, nextY - grabY); break;
		}
	}

	int DebugToolsInterface::EditCharacter(::GUI::Window*, ::GUI::Event* event)
	{
		std::string text;
		entity->GetText(text);
		if (!text.compare("None"))
		{
			char initial[8]; //Only one character and its terminator are originally written.
			initial[0] = char(event->x);
			initial[1] = 0;
			std::string initialText(initial);
			entity->SetText(initialText);
		}
		else entity->AppendCharacter(char(event->x));
		return 1;
	}

	int DebugToolsInterface::EditKey(::GUI::Window* window, ::GUI::Event* event)
	{
		switch (event->x)
		{
		case 28:
		{
			editing = 0;
			std::string text;
			entity->GetText(text);
			if (!text.empty() && text.compare("None"))
			{
				DWORD id = DWORD(atol(text.c_str()));
				if (!GetEntityPtr(id))
				{
					{
						std::string none("None");
						entity->SetText(none);
					}
					DebugServer()->field0x70E8 = 0;
				}
				else
				{
					DebugServer()->field0x70E8 = id;
					DebugWindow* debug = DebugClient()->child0x8C;
					char message[256];
					sprintf(message, "Now debugging: %d", id);
					debug->Write(std::string(message), 128, 128, 0);
				}
			}
			else { entity->SetText(std::string("None")); DebugServer()->field0x70E8 = 0; }
			::GUI::g_Manager.RemoveModal(this);
			break;
		}
		case 1:
		{
			const DWORD selected = DebugServer()->field0x70E8;
			if (!selected)
			{
				std::string none("None");
				entity->SetText(none);
			}
			else
			{
				char text[28];
				sprintf(text, "%d", selected);
				entity->SetText(std::string(text));
			}
			::GUI::g_Manager.RemoveModal(this);
			editing = 0;
			break;
		}
		default:
			return EngineInterface::OnEvent(window, event);
		}
		return 1;
	}

	int DebugToolsInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (GetHidden()) return EngineInterface::OnEvent(window, event);
		if (event->type == 6)
		{
			if (window->type == 1)
			{
		if (sound) sound->Play(0, 0, 0);
		if (!_stricmp(window->GetName(), "ToggleScripts"))
		{
			::GUI::Window* button;
			{
				std::string name("ToggleScripts");
				button = FindWindow(name);
			}
			::GUI::Label* label = NULL;
			if (button)
			{
				std::string name("Label");
				label = static_cast< ::GUI::Label*>(button->FindWindow(name));
			}
			std::string text; label->GetText(text); //Originally required, even when the lookup failed.
			if (text == "Disable Scripts")
			{
				DebugClient()->child0x7C->Queue("disablescripts");
				{ std::string message("Enable Scripts"); label->SetText(message); }
				::GUI::Button* button;
			{
				std::string name("OneRoundBtn");
				button = static_cast< ::GUI::Button*>(FindWindow(name));
			}
				if (button) button->SetEnabled(1);
			}
			else
			{
				DebugClient()->child0x7C->Queue("enablescripts");
				{ std::string message("Disable Scripts"); label->SetText(message); }
				::GUI::Button* button = static_cast< ::GUI::Button*>(FindWindow(std::string("OneRoundBtn")));
				if (button) button->SetEnabled(0);
			}
		}
		else if (!_stricmp(window->GetName(), "OneRoundBtn"))
		{
			::GUI::Button* button = static_cast< ::GUI::Button*>(FindWindow(std::string("OneRoundBtn")));
			if (!button->IsEnabled()) goto Handled;
			DebugClient()->child0x7C->Queue("OneScriptPass");
		}
		else if (!_stricmp(window->GetName(), "ToggleBPsBtn"))
		{
			BYTE old = DebugServer()->field0x70EC;
			::GUI::Window* button;
			{
				std::string name("ToggleBPsBtn");
				button = FindWindow(name);
			}
			::GUI::Label* label = NULL;
			if (button)
			{
				std::string name("Label");
				label = static_cast< ::GUI::Label*>(button->FindWindow(name));
			}
			if (label)
			{
				if (old) { std::string message("Enable Breakpoints"); label->SetText(message); }
				else { std::string message("Disable Breakpoints"); label->SetText(message); }
			}
			DebugServer()->SetBreakpoints(old == 0);
		}
		else if (!_stricmp(window->GetName(), "ResumeBtn"))
		{
			::GUI::Button* button;
			{
				std::string name("ResumeBtn");
				button = static_cast< ::GUI::Button*>(FindWindow(name));
			}
			if (!button->IsEnabled()) goto Handled;
			EnableControls(0);
			F3::NetworkComponent* network = F3::g_pNetworkInstance;
			const DWORD selected = static_cast<Server*>(network->server)->field0x70E8;
			DebugWindow* debug = static_cast<Client*>(network->client)->child0x8C;
			if (!selected && debug)
			{
				std::string message("No entity ID is set for debugging currently.");
				debug->Write(message, 128, 128, 0);
			}
			else
			{
				Script_Small* script = GetEntityPtr(selected)->GetScript();
				if (!script && debug)
				{
					std::string message("That entity does not have a script.");
					debug->Write(message, 128, 128, 0);
				}
				else if (script->StepOver())
				{
					std::string message("Script resumed.");
					debug->Write(message, 0, 128, 0);
				}
				else
				{
					std::string message("Error in resuming script.");
					debug->Write(message, 128, 0, 0);
				}
			}
			goto Handled;
		}
		else if (!_stricmp(window->GetName(), "StepBtn"))
		{
			::GUI::Button* button;
			{
				std::string name("StepBtn");
				button = static_cast< ::GUI::Button*>(FindWindow(name));
			}
			if (!button->IsEnabled()) goto Handled;
			EnableControls(0);
			F3::NetworkComponent* network = F3::g_pNetworkInstance;
			const DWORD selected = static_cast<Server*>(network->server)->field0x70E8;
			DebugWindow* debug = static_cast<Client*>(network->client)->child0x8C;
			if (!selected && debug)
			{
				std::string message("No entity ID is set for debugging currently.");
				debug->Write(message, 128, 128, 0);
			}
			else
			{
				Script_Small* script = GetEntityPtr(selected)->GetScript();
				if (!script && debug)
				{
					std::string message("That entity does not have a script.");
					debug->Write(message, 128, 128, 0);
				}
				else if (!script->StepInto())
				{
					std::string message("Error in stepping script.");
					debug->Write(message, 128, 0, 0);
				}
			}
			goto Handled;
		}
		else if (!_stricmp(window->GetName(), "AbortBtn"))
		{
			::GUI::Button* button;
			{
				std::string name("AbortBtn");
				button = static_cast< ::GUI::Button*>(FindWindow(name));
			}
			if (!button->IsEnabled()) goto Handled;
			EnableControls(0);
			F3::NetworkComponent* network = F3::g_pNetworkInstance;
			const DWORD selected = static_cast<Server*>(network->server)->field0x70E8;
			DebugWindow* debug = static_cast<Client*>(network->client)->child0x8C;
			if (!selected && debug)
			{
				std::string message("No entity ID is set for debugging currently.");
				debug->Write(message, 128, 128, 0);
			}
			else
			{
				Script_Small* script = GetEntityPtr(selected)->GetScript();
				if (!script && debug)
				{
					std::string message("That entity does not have a script.");
					debug->Write(message, 128, 128, 0);
				}
				else if (!script->ContinueDebugging())
				{
					std::string message("Error in aborting script.");
					debug->Write(message, 128, 0, 0);
				}
				else
				{
					std::string message("Script Aborted.");
					debug->Write(message, 128, 0, 0);
				}
			}
			goto Handled;
		}
		else if (!_stricmp(window->GetName(), "ToggleVarsBtn"))
		{
			BYTE old = DebugServer()->field0x70ED;
			::GUI::Window* button;
			{
				std::string name("ToggleVarsBtn");
				button = FindWindow(name);
			}
			::GUI::Label* label = NULL;
			if (button)
			{
				std::string name("Label");
				label = static_cast< ::GUI::Label*>(button->FindWindow(name));
			}
			if (label)
			{
				if (old) { std::string message("Show Locals"); label->SetText(message); }
				else { std::string message("Show Globals"); label->SetText(message); }
			}
			DebugServer()->SetLocalVariables(old == 0);
		}
		else if (!_stricmp(window->GetName(), "SetEntityID"))
		{
			BringToFront(); ::GUI::g_Manager.PushModal(this); editing = 1;
			{ std::string empty(""); entity->SetText(empty); }
		}
			}
		}
		else
		{
			if (event->type == 4 && !editing)
			{
				grabX = event->x - x; grabY = event->y - y;
				oldX = x; oldY = y; dragState = 1;
				BringToFront(); ::GUI::g_Manager.PushModal(this);
			}
			else if (event->type == 5 && !editing)
			{
				::GUI::g_Manager.RemoveModal(this); dragState = 0;
			}
			else if (event->type == 2) Drag(event->x, event->y);
			else if (event->type == 19 && editing) return EditCharacter(window, event);
			else if (event->type == 17 && editing) return EditKey(window, event);
		}
		return EngineInterface::OnEvent(window, event);
	Handled:
		return 1; //Original5B7137; edit-key/character paths preserve their actual helper result above.
	}
}
