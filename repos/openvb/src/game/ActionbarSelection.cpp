#include "game/ActionbarInterface.hpp"
#include "game/ActionbarServices.hpp"
#include "game/GameEntity.hpp"
#include "gui/Window.hpp"
#include "game/FeedbackInterface.hpp"
#include "game/MiniMapInterface.hpp"
#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include <mmsystem.h>

namespace F3
{
	void ActionbarInterface::SelectEntity(DWORD id) //41C9D0.
	{
		if (id != entityID && !GetHidden()) RefreshEntity(id);
	}
	void ActionbarInterface::RefreshEntity(DWORD id) //41C910.
	{
		entityID=id;
		RefreshCompanions();
		GameEntity* entity=dynamic_cast<GameEntity*>(JE::GetEntityPtr(entityID));
		if (entity)
		{
			switch (entity->GetActiveWeaponSlot())
			{
			case 2:
				SetWeaponActive(0,1);
				SetWeaponActive(1,0);
				break;
			case 3:
				SetWeaponActive(0,0);
				SetWeaponActive(1,1);
				break;
			default:
				SetWeaponActive(0,0);
				SetWeaponActive(1,0);
				break;
			}
		}
		if (entityID)
		{
			SetPartyMember(0,entityID);
			SetSelectedPartyMember(0);
		}
		RefreshHealthDigits(1);
		RefreshHealthBar();
		RefreshActionPoints(1);
		ClearStatusButtons();
		RefreshInventory();
	}
	void ActionbarInterface::RefreshCompanions() //416B80.
	{
		if (GetHidden()==1) return;
		GameClient* client=dynamic_cast<GameClient*>(static_cast<JE::Client*>(g_pNetworkInstance->client));
		GameInterfaces* interfaces=client->Interfaces();
		FeedbackInterface* feedback=interfaces->feedback;
		MiniMapInterface* miniMap=interfaces->miniMap;
		switch (value0C)
		{
		case 0:
			feedback->SetHidden(1);
			miniMap->SetHidden(1);
			break;
		case 1:
			feedback->SetHidden(0);
			miniMap->SetHidden(1);
			break;
		case 2:
			feedback->SetHidden(1);
			miniMap->SetHidden(0);
			miniMap->Refresh();
			break;
		}
	}
	void ActionbarInterface::HideCompanions() //416B20, actual parent visibility suffix.
	{
		GameClient* client = dynamic_cast<GameClient*>(static_cast<JE::Client*>(g_pNetworkInstance->client));
		client->Interfaces()->feedback->SetHidden(1);
		client->Interfaces()->miniMap->SetHidden(1);
	}
	void ActionbarInterface::GetWindowRectangle(LONG& x,LONG& y,LONG& width,LONG& height) //416F80.
	{
		rootWindow->GetAbsolutePosition(x,y);
		rootWindow->GetSize(width,height);
	}
	void ActionbarInterface::Tick() //41BFA0, root Window's event0, not an extra draw hook.
	{
		if (popupValue290 && timeGetTime()>popupValue290+500)
		{
			ShowAttackModes(popupValue294);
			popupValue290=0;
		}
		RefreshHealthDigits(0);
		RefreshHealthBar();
		RefreshActionPoints(0);
	}
}
