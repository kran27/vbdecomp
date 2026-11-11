#include "MenuController.hpp"
#include "Window.hpp"

#include <cstring>
#include <stdexcept>

namespace F3
{
	int MenuController::HandleEvent(JE::GUI::MenuNode& node, JE::GUI::MenuEvent& event)
	{
		if (event.type == 6)
		{
			if (!_stricmp(node.name.c_str(), "SingleMultiBtn"))
			{
				singleMulti = singleMulti == 0;
				actions.RefreshSingleMulti(*this);
			}
			else if (!_stricmp(node.name.c_str(), "NewGameBtn"))
			{
				if (static_cast<WORD>(actions.ShiftState()) & 0x8000)
				{
					if (!singleMulti)
					{
						actions.Command("RequestNewGame");
						actions.Hide(layout, 1);
						actions.RemoveModal(&layout);
					}
				}
				else
				{
					actions.FadeOut(1000, 0, 0, 0);
					actions.RemoveModal(&layout);
					actions.Hide(layout, 1);
					actions.ShowNewGame();
					actions.FadeIn(1000, 0, 0, 0);
				}
			}
			else if (!_stricmp(node.name.c_str(), "LoadGameBtn"))
			{
				if (!singleMulti)
					actions.Command("LoadMap");
			}
			else if (_stricmp(node.name.c_str(), "OptionsBtn") &&
				_stricmp(node.name.c_str(), "CinematicsBtn") &&
				!_stricmp(node.name.c_str(), "QuitBtn"))
			{
				actions.QuitApplication();
				actions.RemoveModal(&layout);
			}
		}
		else if (event.type == 17 && &node == mainWindow && event.x == 41)
			actions.Command("ConsoleWindow");
		return JE::GUI::MenuInterface::HandleEvent(node, event);
	}

	SHORT MenuControllerServices::ShiftState() { return ::GetAsyncKeyState(VK_SHIFT); }
	void MenuControllerServices::QuitApplication() { F3::Quit(); }
	void MenuControllerServices::RefreshSingleMulti(MenuController&)
	{ throw std::logic_error("Unrecovered main-menu mode refresh0x440AF0"); }
	void MenuControllerServices::Command(const char*)
	{ throw std::logic_error("Unrecovered game processor command0x580D10"); }
	void MenuControllerServices::Hide(JE::GUI::MenuLayout&, BYTE)
	{ throw std::logic_error("Unrecovered interface hidden setter0x495220"); }
	void MenuControllerServices::FadeOut(DWORD, DWORD, DWORD, DWORD)
	{ throw std::logic_error("Unrecovered game-client fade-out0x588CF0"); }
	void MenuControllerServices::ShowNewGame()
	{ throw std::logic_error("Unrecovered new-game interface0x42F9E0"); }
	void MenuControllerServices::FadeIn(DWORD, DWORD, DWORD, DWORD)
	{ throw std::logic_error("Unrecovered game-client fade-in called at0x440E50"); }
}
