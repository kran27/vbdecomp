#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/OptionsLoadInterface.hpp"
#include "game/OptionsInterface.hpp"
#include "gui/Event.hpp"
#include "gui/Label.hpp"
#include "gui/Window.hpp"
#include <cstdio>
#include <cstring>

SaveSlotOption::SaveSlotOption() : slotLabel(NULL), nameLabel(NULL) //4484D0.
{
}

SaveSlotOption::~SaveSlotOption() //4484F0.
{
	//The actual destructor changes only the vptr, never deletes either Label.
}

namespace F3
{
	OptionsLoadInterface::OptionsLoadInterface(const std::string& name)
		: JE::EngineInterface(name), GameInterface(name) //448C40;strings/list construct before resource loading.
	{
		LoadResource("Options_Load.INT");
		rootWindow = FindWindow("picBackground");
		rowSkin = "DefaultInterface.ini";
		rowFontName = "arial";
		rowFontSize = 10;
		rowFontFlags = 0;
		rowX = 0;
		rowY = 0;
		rowWidth = 100;
		rowHeight = 10;
		//Original erases the fresh empty list here using the actual STL API.
		rows.clear();
		informationLabels[0] = static_cast<::GUI::Label*>(FindWindow("lblSaveGameInfo01"));
		informationLabels[1] = static_cast<::GUI::Label*>(FindWindow("lblSaveGameInfo02"));
		informationLabels[2] = static_cast<::GUI::Label*>(FindWindow("lblSaveGameInfo03"));
		::GUI::Label* prototype = static_cast<::GUI::Label*>(FindWindow("lblGameSlot"));
		if (prototype)
		{
			prototype->GetAbsolutePosition(rowX, rowY);
			prototype->GetSize(rowWidth, rowHeight);
			prototype->GetSkin(rowSkin);
			prototype->GetFont(rowFontName, &rowFontFlags, &rowFontSize, NULL);
			prototype->SetHidden(1);
		}
		//The original clears all three only when all three required names resolve.
		if (informationLabels[0] && informationLabels[1] && informationLabels[2])
		{
			informationLabels[0]->SetText(std::string(""));
			informationLabels[1]->SetText(std::string(""));
			informationLabels[2]->SetText(std::string(""));
		}
		BuildRows();
	}

	OptionsLoadInterface::~OptionsLoadInterface() //4486E0.
	{
		for (std::list<SaveSlotOption*>::iterator row = rows.begin(); row != rows.end(); ++row)
			if (*row)
				delete *row;
		rows.clear();
		//Generated Labels remain attached to rootWindow for original GUI selective
		//child cleanup. Do not repair its recorded HeapTracker omission here.
	}

	void OptionsLoadInterface::BuildRows() //448900.
	{
		int slot = 0;
		for (LONG offset = 0; offset < 495; offset += 33)
		{
			SaveSlotOption* row = new SaveSlotOption;
			::GUI::Label* label = new ::GUI::Label(3);
			row->slotLabel = label;
			rootWindow->AddChild(label);
			label->ApplySkin(rowSkin);
			label->SetFont(rowFontName, rowFontFlags, rowFontSize, 1);
			label->SetTextColor(0xFF008000);
			char text[256];
			std::sprintf(text, "[ GAME SLOT %02d ] ", ++slot);
			label->SetText(text);
			label->Resize(rowWidth, rowHeight);
			label->SetAbsolutePosition(rowX, rowY + offset);
			label->SetValue3B(0);

			label = new ::GUI::Label(3);
			row->nameLabel = label;
			rootWindow->AddChild(label);
			label->ApplySkin(rowSkin);
			label->SetFont(rowFontName, rowFontFlags, rowFontSize, 1);
			label->SetTextColor(0xFF008000);
			label->SetText("Unassigned");
			label->Resize(rowWidth, rowHeight);
			label->SetAbsolutePosition(rowX, rowY + rowHeight + offset);
			label->SetValue3B(0);
			rows.push_back(row);
		}
	}

	void OptionsLoadInterface::SelectSlot(SaveSlotOption* selected) //448520.
	{
		for (std::list<SaveSlotOption*>::iterator entry = rows.begin();
			entry != rows.end(); ++entry)
		{
			SaveSlotOption* row = *entry;
			//Original tests row only for the match, then dereferences it in either arm.
			if (row && row == selected)
			{
				row->nameLabel->SetTextColor(0xFF000080);
				row->slotLabel->SetTextColor(0xFF000080);
			}
			else
			{
				row->nameLabel->SetTextColor(0xFF008000);
				row->slotLabel->SetTextColor(0xFF008000);
			}
		}
	}

	void OptionsLoadInterface::ClearSelection() //448590.
	{
		SelectSlot(NULL);
	}

	int OptionsLoadInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event) //4485A0.
	{
		if (event->type == 6)
		{
			if (_stricmp(window->GetName(), "btnLoad") == 0)
			{
				//Recognized original prototype:there is no Load implementation.
			}
			else if (_stricmp(window->GetName(), "btnSave") == 0)
			{
				//Recognized original prototype:there is no Save implementation.
			}
			else if (_stricmp(window->GetName(), "btnDelete") == 0)
			{
				//Recognized original prototype:there is no Delete implementation.
			}
			else if (_stricmp(window->GetName(), "btnCancel") == 0)
			{
				OptionsInterface* options = static_cast<GameInterfaces*>(static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xA0)->options;
				options->CloseChildren();
				options->SetHidden(1);
			}
		}
		else if (event->type == 4)
		{
			for (std::list<SaveSlotOption*>::iterator entry = rows.begin();
				entry != rows.end(); ++entry)
			{
				SaveSlotOption* row = *entry;
				if (row && (window == row->nameLabel || window == row->slotLabel))
				{
					SelectSlot(row);
					break;
				}
			}
		}
		else if (event->type == 17 && window == rootWindow)
		{
			switch (event->x)
			{
			case 1:
				{
					OptionsInterface* options = static_cast<GameInterfaces*>(static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xA0)->options;
					options->CloseChildren();
					options->SetHidden(1);
					return 1;
				}
			}
		}
		return JE::EngineInterface::OnEvent(window, event);
	}
}
