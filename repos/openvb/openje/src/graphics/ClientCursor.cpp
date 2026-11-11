#include <winsock2.h>
#include "ClientCursor.hpp"
#include "ClientCursorActions.hpp"
#include "ClientDebugReport.hpp"
#include "SystemInterface.hpp"
#include "gui/Interface.hpp"
#include "gui/Picture.hpp"
#include "gui/Manager.hpp"
#include "gui/Label.hpp"
#include "entity/Entity.hpp"
#include "game/GameCreature.hpp"
#include "game/GameEntity.hpp"
#include "game/GameSkillMenu.hpp"
#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "startup/Window.hpp"
#include <stdexcept>
#include <cstring>

namespace
{
	//Explicit snapshot/window host adaptation. The original noarg game update
	//uses Manager::ProcessCursor4B2D40 and its real FindCursorWindow owner.
	BYTE GUICursor(HWND window)
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(window,&point);
		::GUI::Manager& manager = ::GUI::g_Manager;
		const LONG x = static_cast<LONG>(double(point.x)/manager.scaleX);
		const LONG y = static_cast<LONG>(double(point.y)/manager.scaleY);
		::GUI::Window* hit = manager.capturedWindow;
		if (!hit)
		{
			for (std::list<::GUI::Interface*>::iterator i = manager.interfaces.begin(); i != manager.interfaces.end(); ++i)
			{
				hit = (*i)->FindAt(x,y,0);
				if (hit) break;
			}
		}
		for (::GUI::Window* ancestor = hit; ancestor; ancestor = ancestor->GetParent())
		{
			::GUI::Interface* owner = ancestor->GetOwner();
			if (owner) return owner->OnCursor(hit,point.x,point.y);
		}
		return 0;
	}
	const char* const CursorNames[17] = {
		"DefaultCursor.tga","DefaultCursor.tga","DefaultCursor.tga","UseCursor.tga",
		"TalkCursor.tga","DefaultCursor.tga","DefaultCursor.tga","DefaultCursor.tga",
		"DefaultCursor.tga","UseCursor.tga","UseCursor.tga","DefaultCursor.tga",
		"DefaultCursor.tga","PickupCursor.tga","DefaultCursor.tga","AttackCursor.tga","TargetCursor.tga"
	};
}

namespace JE
{
	CursorManager::CursorManager() //591700.
	{
		cursorInterface = NULL;
		picture = NULL;
		hotspotX = 0;
		hotspotY = 0;
		imageName = "";
	}
	CursorManager::~CursorManager() { ShowSystemCursor(); }
	void CursorManager::OnCleanup() {}
	void CursorManager::OnUnused() {}
	void CursorManager::ShowSystemCursor() { ::ShowCursor(TRUE); }
	void CursorManager::Initialize() //591780.
	{
		::ShowCursor(FALSE);
		if (!cursorInterface) cursorInterface = new ::GUI::Interface("Cursor");
		if (!picture)
		{
			Gfx_Viewport* viewport = g_GfxSystem->PostViewport();
			picture = new ::GUI::Picture(2);
			picture->SetName("Cursor");
			picture->SetFrameVisible(0);
			picture->Resize(32,32);
			picture->ApplySkin("DefaultInterface.ini");
			picture->SetHidden(0);
			picture->SetValue3B(1);
			picture->SetViewport(viewport);
			cursorInterface->AddWindow(picture);
		}
		SetVisible(0);
	}
	void CursorManager::SetVisible(BYTE visible) { if (picture) picture->SetHidden(visible == 0); }
	void CursorManager::RequestImage(const char* name, LONG x, LONG y, BYTE center) //5919D0.
	{
		if (!_stricmp(name,imageName.c_str())) return;
		imageName = name;
		picture->LoadTexture(std::string(name),0,0,0,-1,-1,1);
		const LONG width = picture->pictureWidth, height = picture->pictureHeight;
		picture->Resize(width,height);
		if (width > 1 && height > 1) picture->SetSourceRect(0.5f,0.5f,float(width)-1.0f,float(height)-1.0f);
		picture->SetTileMode(4);
		if (center)
		{
			hotspotX = x+width/2;
			hotspotY = y+height/2;
		}
		else
		{
			hotspotX = x;
			hotspotY = y;
		}
	}
	void CursorManager::GetImage(std::string& output) { output = imageName; }
	BYTE CursorManager::ReleaseInput(const Input::InputEvent&) { return 0; } //591690, RET4.
	BYTE CursorManager::ReleaseInput(ClientInputState& input, const Input::InputEvent& event, HWND window)
	{
		//The borrowed workspace entry has no original virtual slot. Dispatch its
		//explicit snapshot overload without replacing the canonical event-only ABI.
		F3::GameCursorManager* game = dynamic_cast<F3::GameCursorManager*>(this);
		return game ? game->ReleaseInput(input,event,window) : 0;
	}
	void CursorManager::UpdateFrame() //5915E0.
	{
		if (!cursorInterface || !picture) return;
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(F3::g_window,&point);
		float scaleX,scaleY;
		::GUI::g_Manager.GetScale(scaleX,scaleY);
		picture->SetAbsolutePosition(static_cast<LONG>(double(LONG(DWORD(point.x)-DWORD(hotspotX)))/scaleX),
			static_cast<LONG>(double(LONG(DWORD(point.y)-DWORD(hotspotY)))/scaleY));
	}
	void CursorManager::UpdateFrame(ClientInputState&, HWND window) //Explicit host adaptation.
	{
		if (!cursorInterface || !picture) return;
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(window,&point);
		float scaleX,scaleY;
		::GUI::g_Manager.GetScale(scaleX,scaleY);
		picture->SetAbsolutePosition(static_cast<LONG>(double(LONG(DWORD(point.x)-DWORD(hotspotX)))/scaleX),
			static_cast<LONG>(double(LONG(DWORD(point.y)-DWORD(hotspotY)))/scaleY));
	}
}

namespace F3
{
	GameCursorManager::GameCursorManager() //436330.
		: pendingImage(0), pendingX(0), pendingY(0), pendingCenter(0)
	{
		context = 0;
		subcontext = 0;
		draggedItemID = 0;
		choices.clear();
		tooltipWindow = NULL;
		tooltipLabel = NULL;
		tooltipPending = 0;
		tooltipEntityID = 0;
		tooltipTime = 0;
		//Original436330 leaves draggedOwnerID/savedX/savedY unwritten.
	}
	GameCursorManager::~GameCursorManager() {}
    void GameCursorManager::ClearDragChoices() { choices.clear(); }
	void GameCursorManager::ShowSystemCursor() //4361E0.
	{
		JE::CursorManager::ShowSystemCursor();
	}
	void GameCursorManager::OnUnused() //437B40.
	{
		pendingImage = 0;
		pendingCenter = 0;
		savedName = "";
		context = 0;
		subcontext = 0;
		draggedItemID = 0;
		ClearDragChoices();
		QueueContextImage(1,0,0);
	}
	void GameCursorManager::Initialize() //437BB0.
	{
		JE::CursorManager::Initialize();
		InitializeTooltip();
		QueueContextImage(1,0,0);
		ClearDragChoices();
		SetVisible(1);
	}
	void GameCursorManager::InitializeTooltip() //4364C0.
	{
		Gfx_Viewport* viewport = g_GfxSystem->PostViewport();
		LONG x,y;
		picture->GetAbsolutePosition(x,y);
		tooltipWindow = new ::GUI::Window(0);
		tooltipWindow->ApplySkin(std::string("quickslot.ini"));
		tooltipWindow->SetAlpha(150);
		tooltipWindow->Resize(400,19);
		tooltipWindow->SetAbsolutePosition(x,y-20);
		tooltipWindow->SetValue3B(1);
		tooltipWindow->SetViewport(viewport);
		tooltipWindow->SetHidden(1);
		tooltipLabel = new ::GUI::Label(3);
		tooltipLabel->ApplySkin(std::string("quickslot.ini"));
		tooltipLabel->Resize(400,19);
		tooltipLabel->SetFont(std::string("fallouty"),0,9,1);
		tooltipLabel->SetTextColor(0xFF808080);
		tooltipLabel->SetWrap(0);
		tooltipLabel->SetAlignment(0);
		tooltipLabel->SetAbsolutePosition(x+8,y-16);
		tooltipLabel->SetViewport(viewport);
		tooltipLabel->SetText(std::string("Tool Tip"));
		tooltipLabel->SetHidden(0);
		tooltipWindow->AddChild(tooltipLabel);
		picture->AddChild(tooltipWindow);
	}
	void GameCursorManager::InitializeDefaultCursorState() //437BC0 suffix only.
	{
		QueueContextImage(1,0,0);
		choices.clear();
		SetVisible(1);
	}
	void GameCursorManager::RequestImage(const char* name, LONG x, LONG y, BYTE center) //437AB0.
	{
		if (pendingImage && x == pendingX && y == pendingY && !_stricmp(pendingName.c_str(),name)) return;
		pendingCenter = center;
		pendingImage = 1;
		pendingName = name;
		pendingX = x;
		pendingY = y;
	}
	void GameCursorManager::GetImage(std::string& output)
	{ if (pendingImage) output = pendingName; else JE::CursorManager::GetImage(output); }
	void GameCursorManager::QueueContextImage(DWORD value, LONG x, LONG y)
	{
		RequestImage(CursorNames[value],x,y,0);
	}
	void GameCursorManager::SetSubcontext(DWORD value) { subcontext = value; QueueContextImage(value,0,0); }
	void GameCursorManager::SetContext(DWORD value) //436AD0.
	{
		context = value;
		LONG x = 0;
		LONG y = 0;
		if (value == 1) subcontext = 0;
		else if (value == 5) { x = 4; y = 4; }
		else if (value == 15 || value == 16) { x = 15; y = 15; }
		QueueContextImage(value,x,y);
		savedName = pendingName;
		savedX = pendingX;
		savedY = pendingY;
		UpdateFrame();
	}
	void GameCursorManager::SetContext(DWORD value, JE::ClientInputState& input, HWND window) //Explicit host adaptation.
	{
		context = value;
		LONG hotspot = 0;
		if (value == 1) subcontext = 0;
		else if (value == 5) hotspot = 4;
		else if (value == 15 || value == 16) hotspot = 15;
		QueueContextImage(value,hotspot,hotspot);
		savedName = pendingName;
		savedX = pendingX;
		savedY = pendingY;
		UpdateFrame(input,window);
	}
	void GameCursorManager::UpdateTooltip(LONG id) //436840.
	{
		const LONG requestedID = id;
		if (id != tooltipEntityID)
		{
			if (!id)
			{
				tooltipLabel->SetText(std::string(""));
			}
			else
			{
				JE::Entity* engineEntity = JE::GetEntityPtr(id);
				GameEntity* entity = dynamic_cast<GameEntity*>(engineEntity);
				if (entity)
				{
					{
						std::string name = entity->GetDisplayName();
						if (!name.empty())
						{
							tooltipLabel->SetText(name);
							LONG width = tooltipLabel->GetTextWidth();
							LONG previousWidth;
							tooltipWindow->GetSize(previousWidth,id);
							tooltipWindow->Resize(width+16,id);
							tooltipWindow->SetHidden(1);
							tooltipPending = 1;
							tooltipTime = timeGetTime();
						}
						else
						{
							tooltipLabel->SetText(std::string(""));
							tooltipWindow->SetHidden(1);
							tooltipPending = 0;
						}
					}
					tooltipEntityID = requestedID;
					return;
				}
				tooltipLabel->SetText(std::string(""));
			}
			tooltipWindow->SetHidden(1);
			tooltipPending = 0;
			tooltipEntityID = requestedID;
			return;
		}
		if (tooltipPending && timeGetTime() > tooltipTime+250)
		{
			tooltipWindow->SetHidden(0);
			tooltipPending = 0;
		}
	}
	void GameCursorManager::UpdateFrame() //437BE0.
	{
		BYTE requestedTooltip = 0;
		if (!::GUI::g_Manager.ProcessCursor(F3::g_window))
		{
			JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
			if (!client->field0x110) SetSubcontext(0);
			else if (context == 1)
			{
				const DWORD hoveredID = client->field0xDC;
				if (!hoveredID) SetSubcontext(hoveredID);
				else
				{
					const DWORD selectedID = client->field0xE0;
					if (!selectedID || selectedID == hoveredID)
					{
						SetSubcontext(0);
						if (selectedID == hoveredID)
						{ UpdateTooltip(hoveredID); requestedTooltip = 1; }
					}
					else
					{
						JE::Entity* selected = JE::GetEntityPtr(selectedID);
						JE::Entity* hovered = JE::GetEntityPtr(hoveredID);
						if (selected && hovered)
						{
							SetSubcontext(hovered->GetContextAction(selected));
							UpdateTooltip(hoveredID);
							requestedTooltip = 1;
						}
						else
						{
							client->field0xE0 = 0;
							SetSubcontext(0);
						}
					}
				}
			}
			else if (!pendingImage && !savedName.empty()) RequestImage(savedName.c_str(),savedX,savedY,0);
			if (pendingImage) { savedName = pendingName; savedX = pendingX; savedY = pendingY; }
		}
		if (pendingImage)
		{
			JE::CursorManager::RequestImage(pendingName.c_str(),pendingX,pendingY,pendingCenter);
			pendingImage = 0;
			pendingCenter = 0;
		}
		if (!requestedTooltip) UpdateTooltip(0);
		JE::CursorManager::UpdateFrame();
	}
	void GameCursorManager::UpdateFrame(JE::ClientInputState& input, HWND window) //Explicit host adaptation.
	{
		BYTE requestedTooltip = 0;
		if (!GUICursor(window))
		{
			if (!input.enabled) SetSubcontext(0);
			else if (context != 1)
			{
				if (!pendingImage && !savedName.empty()) RequestImage(savedName.c_str(),savedX,savedY,0);
			}
			else if (input.hoveredEntityID)
			{
				if (!input.selectedEntityID || input.selectedEntityID == input.hoveredEntityID)
				{
					SetSubcontext(0);
					if (input.selectedEntityID == input.hoveredEntityID)
					{ UpdateTooltip(input.hoveredEntityID); requestedTooltip = 1; }
				}
				else
				{
					JE::Entity* selected = JE::GetEntityPtr(input.selectedEntityID);
					JE::Entity* hovered = JE::GetEntityPtr(input.hoveredEntityID);
					if (selected && hovered)
					{
						SetSubcontext(hovered->GetContextAction(selected));
						UpdateTooltip(input.hoveredEntityID);
						requestedTooltip = 1;
					}
					else
					{
						input.selectedEntityID = 0;
						SetSubcontext(0);
					}
				}
			}
			else SetSubcontext(0);
			if (pendingImage) { savedName = pendingName; savedX = pendingX; savedY = pendingY; }
		}
		if (pendingImage)
		{
			JE::CursorManager::RequestImage(pendingName.c_str(),pendingX,pendingY,pendingCenter);
			pendingImage = 0;
			pendingCenter = 0;
		}
		if (!requestedTooltip) UpdateTooltip(0);
		JE::CursorManager::UpdateFrame(input,window);
	}
	BYTE GameCursorManager::LeftPress(LONG, LONG) { return 0; }
	BYTE GameCursorManager::RightPress(LONG, LONG) { return 0; }

	BYTE GameCursorManager::ReleaseInput(const JE::Input::InputEvent& event) //437D80.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		if (!client->field0x110) return 0;
		BYTE handled = 0;
		if (event.category == 2)
		{
			if (event.subtype == 1)
			{
				if (!event.button) return LeftPress(event.x,event.y);
				if (event.button == 2) return RightPress(event.x,event.y);
			}
			else if (event.subtype == 2)
			{
				if (!event.button) return LeftRelease(event.x,event.y);
				if (event.button == 2 && (event.modifiers & 1))
					return DebugPrintEntityInfo(event.x,event.y);
				if (event.button == 2) handled = RightRelease(event.x,event.y);
			}
		}
		return handled;
	}

	BYTE GameCursorManager::RightRelease(LONG x, LONG y) //436FC0.
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		const DWORD selectedID = client->field0xE0;
		const DWORD hoveredID = client->field0xDC;
		JE::Entity* selected = JE::GetEntityPtr(selectedID);
		JE::Entity* hovered = JE::GetEntityPtr(hoveredID);
		client = static_cast<JE::Client*>(g_pNetworkInstance->client); //436FE8: reload before either entity gate.
		if (!selected || !hovered) return 0;
		client->RequestContextMenu(selectedID,hoveredID,x,y);
		SetContext(1);
		return 1;
	}
	BYTE GameCursorManager::ReleaseInput(JE::ClientInputState& input, const JE::Input::InputEvent& event, HWND window)
	{
		if (!input.enabled || event.category != 2 || event.subtype != 2) return 0;
		if (!event.button) return LeftRelease(input,event.x,event.y,window);
		if (event.button != 2) return 0;
		if (event.modifiers & 1)
			return DebugPrintEntityInfo(event.x,event.y);
		const DWORD selectedID = input.selectedEntityID, hoveredID = input.hoveredEntityID;
		JE::Entity* selected = JE::GetEntityPtr(selectedID);
		JE::Entity* hovered = JE::GetEntityPtr(hoveredID);
		if (!selected || !hovered) return 0;
		Actions().OpenContextMenu(selectedID,hoveredID,event.x,event.y);
		SetContext(1,input,window);
		return 1;
	}
}
