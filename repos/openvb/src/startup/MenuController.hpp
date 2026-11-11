#pragma once

#include "gui/MenuInterface.hpp"

namespace F3
{
	class MenuController;
	class MenuControllerServices : public JE::GUI::MenuInterfaceServices
	{
	public:
		virtual SHORT ShiftState();
		virtual void RefreshSingleMulti(MenuController& menu);
		virtual void Command(const char* text);
		virtual void Hide(JE::GUI::MenuLayout& layout, BYTE hidden);
		virtual void RemoveModal(JE::GUI::MenuLayout* layout) = 0;
		virtual void FadeOut(DWORD duration, DWORD arg1, DWORD arg2, DWORD arg3);
		virtual void ShowNewGame();
		virtual void FadeIn(DWORD duration, DWORD arg1, DWORD arg2, DWORD arg3);
		virtual void QuitApplication();
	};

	class MenuController : public JE::GUI::MenuInterface
	{
	public:
		MenuController(JE::GUI::MenuLayout& layout, JE::GUI::MenuNode* mainWindow,
			MenuControllerServices& services) : MenuInterface(layout, services), singleMulti(0),
			mainWindow(mainWindow), actions(services) {}
		int HandleEvent(JE::GUI::MenuNode& node, JE::GUI::MenuEvent& event); //0x440CE0.
		BYTE singleMulti; // Actual Mainmenu constructor byte+4.
		JE::GUI::MenuNode* mainWindow; // Same canonical node graph, borrowed.
		MenuControllerServices& actions;
	};
}
