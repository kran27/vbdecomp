#include <winsock2.h>
#include "game/PipBoyAddNoteInterface.hpp"
#include "game/PipBoyInterface.hpp"
#include "game/GameInterfaces.hpp"
#include "game/AreaMapResources.hpp"
#include "gui/Edit.hpp"
#include "gui/Event.hpp"
#include "gui/Manager.hpp"
#include "gui/Picture.hpp"
#include "gui/Win32Input.hpp"
#include "graphics/StringFormat.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"
#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include <cstdlib>
#include <cstring>

namespace F3
{
	BYTE NoteTextIsBlank(const char* text)
	{
		//The reached original checks only this exact four-character set. The
		//real CRT supplies membership/length handling; this adapter claims no
		//standalone4A1170 code match or general string-library reconstruction.
		return !text || std::strspn(text," \n\t\r") == std::strlen(text);
	}
	PipBoyAddNoteInterface::PipBoyAddNoteInterface(const std::string& name)
		: JE::EngineInterface(name), GameInterface(name) //44DA10; required separate registered interface.
	{
		LoadResource("AreaMapNote.INT"); //Its actual Edit4 child is mandatory.
		pipBoy = NULL;
		areaMap = NULL;
		note = NULL;
		iconIndex = 1;
	}

	PipBoyAddNoteInterface::~PipBoyAddNoteInterface() //44A560.
	{
		//Only qualified GameInterface/EngineInterface lifetime; the note/map are borrowed.
	}

	std::string PipBoyAddNoteInterface::GetIconTextureName(LONG index) //44B2A0.
	{
		std::string name;
		JE::FormatString(name,"picIcon%02d",index);
		::GUI::Picture* icon = static_cast< ::GUI::Picture*>(FindWindow(name));
		if (icon) icon->GetTextureName(name);
		return name;
	}
	DWORD PipBoyAddNoteInterface::GetIconTexture(LONG index) //44AD50.
	{
		std::string name;
		JE::FormatString(name,"picIcon%02d",index);
		::GUI::Picture* icon = static_cast< ::GUI::Picture*>(FindWindow(name));
		if (!icon) return 0;
		DWORD handle;
		icon->GetTextureHandle(handle);
		return handle;
	}
	void PipBoyAddNoteInterface::RefreshText() //44DB10.
	{
		::GUI::Edit* edit = static_cast< ::GUI::Edit*>(FindWindow("edtNote"));
		if (note)
		{
			if (!note->text.empty()) edit->SetText(note->text);
			else edit->SetStringReference(DWORD(note->value30));
		}
		else edit->SetText(std::string(""));
		std::string text;
		edit->GetText(text);
		edit->SetSelection(LONG(text.size()),LONG(text.size()));
		edit->SetScroll(0,0);
	}
	void PipBoyAddNoteInterface::SetPopupPosition(LONG x, LONG y) //44DCB0.
	{
		::GUI::Window* root = FindWindow("winAreaMapNote");
		root->SetAbsolutePosition(x,y);
	}
	void PipBoyAddNoteInterface::RefreshIcons() //44DD70.
	{
		LONG index=1;
		std::string name;
		for (;;++index)
		{
			JE::FormatString(name,"picIcon%02d",index);
			::GUI::Window* icon = FindWindow(name.c_str());
			if (!icon) break;
			icon->SetValue3B(0);
			if (index==iconIndex) icon->SetAlpha(255);
			else icon->SetAlpha(128);
			icon->SetValue60(1);
			icon->SetMaterialModes(1);
		}
	}
	void PipBoyAddNoteInterface::Open(PipBoyInterface* parent, JE::AreaMap* map, JE::AreaMapNote* target, BYTE readOnly) //44FC70.
	{
		pipBoy = parent;
		areaMap = map;
		note = target;
		iconIndex = 0;
		if (!target->name.empty())
		{
			LONG index=1;
			std::string name;
			for (;;++index)
			{
				JE::FormatString(name,"picIcon%02d",index);
				::GUI::Picture* icon = static_cast< ::GUI::Picture*>(FindWindow(name.c_str()));
				if (!icon) break;
				icon->GetTextureName(name);
				if (!_stricmp(note->name.c_str(),name.c_str()))
				{
					iconIndex = index;
					break;
				}
			}
		}
		RefreshText();
		RefreshIcons();
		::GUI::Edit* edit = static_cast< ::GUI::Edit*>(FindWindow("edtNote"));
		edit->readOnly = readOnly;
		if (readOnly) edit->SetTextColor(DWORD(-12781568));
		else
		{
			edit->SetTextColor(0xFF808080);
			edit->ClampSelection(1);
		}
		edit->caretColor = 0x80FFFFFF;
		edit->selectionColor = 0x800000FF;
		edit->editGeometryDirty = 1;
		BringToFront();
		SetHidden(0);
	}
	int PipBoyAddNoteInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event) //453EB0 ->453540.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		PipBoyInterface* owner = static_cast<GameInterfaces*>(client->child0xA0)->pipBoy;
		::GUI::Edit* edit = static_cast< ::GUI::Edit*>(FindWindow("edtNote"));
		if (event->type==4 || event->type==7)
		{
			if (window->type==2 && std::strlen(window->GetName())==9 &&
				!_strnicmp(window->GetName(),"picIcon",7))
			{
				iconIndex = std::atol(window->GetName()+7);
				if (note)
				{
					std::string textureName = GetIconTextureName(iconIndex);
					note->name = textureName;
					note->LoadTexture();
					Gfx_Texture* texture = g_GfxSystem->LookupTexture(note->texture);
					if (texture)
					{
						note->value28 = texture->frameCount>0 ? texture->frames[0].width : 0;
						note->value2C = texture->frameCount>0 ? texture->frames[0].height : 0;
						note->value20 = (texture->frameCount>0 ? texture->frames[0].width : 0)>>1;
						note->value24 = (texture->frameCount>0 ? texture->frames[0].height : 0)>>1;
					}
					if (areaMap) areaMap->changes |= 2;
				}
				RefreshIcons();
			}
		}
		else if (event->type==17 && !GetHidden())
		{
			::GUI::Window* root = FindWindow("winAreaMapNote");
			if (window==root)
			{
				const LONG key = event->x;
				if (key==1)
				{
					SetHidden(1);
					::GUI::g_Manager.RemoveModal(this);
					owner->FinishNoteEdit(0,0);
				}
				else if (key==28 || key==156 || key==284)
				{
					static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x78->FindKey("Enter");
					SetHidden(1);
					::GUI::g_Manager.RemoveModal(this);
					std::string text;
					edit->GetText(text);
					if (JE::IsStringBlank(text.c_str(), " \n\t\r")) owner->FinishNoteEdit(0,1);
					else
					{
						owner->SetNoteText(text);
						owner->FinishNoteEdit(1,0);
					}
				}
			}
		}
		else if (event->type==2)
		{
			if (::GUI::g_Manager.FrontModal()!=this && pipBoy)
				pipBoy->OnEvent(window,event);
		}
		return JE::EngineInterface::OnEvent(window,event);
	}
}
