#include <winsock2.h>
#include "TextEditInterface.hpp"
#include "FontTestInterface.hpp"
#include "gui/Edit.hpp"
#include "gui/Event.hpp"
#include "gui/Manager.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace JE
{
	TextEditInterface::TextEditInterface(const std::string& name) : EngineInterface(name)
	{
		LoadResource("AddNote.INT");
		main = FindWindow("AddNoteWin");
		edit = static_cast<::GUI::Edit*>(FindWindow("NoteEditText"));
		edit->SetWrap(1);
		mode = 0;
		clearOnFirstKey = 1;
	}

	TextEditInterface::~TextEditInterface() {}

	void TextEditInterface::Clear()
	{
		edit->SetText("");
	}

	void TextEditInterface::SetMode(DWORD value)
	{
		FontTestInterface* owner = static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x98;
		std::string face, text;
		owner->text4->GetText(text);
		owner->text4->GetFont(face, NULL, NULL, NULL);
		LONG size = owner->text1->GetFontSize();
		DWORD color = owner->text1->GetTextColor();
		DWORD alpha = static_cast<BYTE>(owner->main->GetAlpha());
		const int red = BYTE(color >> 16), green = BYTE(color >> 8), blue = BYTE(color);
		mode = value;
		clearOnFirstKey = 1;
		char buffer[256];
		switch (value)
		{
		case 1: edit->SetText(face); break;
		case 2:
			sprintf(buffer, "%d", size);
			edit->SetText(std::string(buffer));
			break;
		case 3: edit->SetText(text); break;
		case 4:
			sprintf(buffer, "%d", int(alpha));
			edit->SetText(std::string(buffer));
			break;
		case 5:
			sprintf(buffer, "%d %d %d", red, green, blue);
			edit->SetText(std::string(buffer));
			break;
		}
	}

	void TextEditInterface::Apply()
	{
		FontTestInterface* owner = static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x98;
		std::string face, text;
		owner->text4->GetText(text);
		owner->text4->GetFont(face, NULL, NULL, NULL);
		LONG size = owner->text1->GetFontSize();
		DWORD color = owner->text1->GetTextColor();
		owner->main->GetAlpha(); //Original performs this call even though its result is unused.
		int red = BYTE(color >> 16), green = BYTE(color >> 8), blue = BYTE(color);
		std::string value;
		edit->GetText(value);
		switch (mode)
		{
		case 1:
			owner->text1->SetFont(value, 0, size, 1);
			owner->text2->SetFont(value, 1, size, 1);
			owner->text3->SetFont(value, 2, size, 1);
			owner->text4->SetFont(value, 0, size, 1);
			break;
		case 2:
			owner->text1->SetFont(face, 0, atol(value.c_str()), 1);
			owner->text2->SetFont(face, 1, atol(value.c_str()), 1);
			owner->text3->SetFont(face, 2, atol(value.c_str()), 1);
			owner->text4->SetFont(face, 0, atol(value.c_str()), 1);
			break;
		case 3: owner->text4->SetText(value); break;
		case 4: owner->main->SetAlpha(static_cast<BYTE>(atol(value.c_str()))); break;
		case 5:
			sscanf(value.c_str(), "%d %d %d", &red, &green, &blue);
			owner->text1->SetTextColor(D3DCOLOR_XRGB(red, green, blue));
			owner->text2->SetTextColor(D3DCOLOR_XRGB(red, green, blue));
			owner->text3->SetTextColor(D3DCOLOR_XRGB(red, green, blue));
			owner->text4->SetTextColor(D3DCOLOR_XRGB(red, green, blue));
			break;
		}
	}

	int TextEditInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (event->type == 6)
		{
			if (window->type == 1)
			{
				if (!_stricmp(window->GetName(), "AddBtn"))
				{
					Apply();
					SetHidden(1);
					::GUI::g_Manager.RemoveModal(this);
				}
				else if (!_stricmp(window->GetName(), "ClearBtn")) Clear();
				else if (!_stricmp(window->GetName(), "CancelBtn"))
				{
					SetHidden(1);
					::GUI::g_Manager.RemoveModal(this);
				}
			}
		}
		else if (event->type == 17 && window == main)
		{
			switch (event->x)
			{
			case 1:
				SetHidden(1);
				::GUI::g_Manager.RemoveModal(this);
				break;
			case 28:
				Apply();
				SetHidden(1);
				::GUI::g_Manager.RemoveModal(this);
				break;
			default:
				if (clearOnFirstKey)
				{
					Clear();
					clearOnFirstKey = 0;
				}
				break;
			}
		}
		return EngineInterface::OnEvent(window, event);
	}
}
