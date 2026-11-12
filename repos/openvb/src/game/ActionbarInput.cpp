#include "game/ActionbarInterface.hpp"
#include "game/ActionbarServices.hpp"
#include "game/InventoryInputServices.hpp"
#include "game/FeedbackInterface.hpp"
#include "game/GameCreature.hpp"
#include "game/GameInventory.hpp"
#include "game/GameItem.hpp"
#include "game/GameWeapon.hpp"
#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "graphics/ClientCursor.hpp"
#include "gui/Button.hpp"
#include "gui/Picture.hpp"
#include "gui/Label.hpp"
#include "gui/Event.hpp"
#include "network/Packet.hpp"
#include <cstdio>
#include <mmsystem.h>
#include <stdexcept>

namespace F3
{
	namespace
	{
		BYTE sneaking=0; //Original single game global707740; not an actor trait mutation.
		void SetCursorContext(GameClient* client,DWORD context)
		{
			GameCursorManager* cursor = static_cast<GameCursorManager*>(client->child0xA4);
			InventoryInputServices::SetContext(*cursor,context);
		}
		void SetCursorContext(DWORD context)
		{
			SetCursorContext(dynamic_cast<GameClient*>(g_pNetworkInstance->client),context);
		}
	}
	void ActionbarInterface::PrimaryWeaponClick(DWORD index) //41C790.
	{
		GameInventory* inventory=GetInventory();
		if (!inventory) return;
		GameItem* item=NULL;
		if (index==0) item=inventory->GetItem(2);
		else if (index==1) item=inventory->GetItem(3);
		GameWeapon* weapon=dynamic_cast<GameWeapon*>(item);
		GameClient* client=dynamic_cast<GameClient*>(g_pNetworkInstance->client);
		GameCreature* creature=dynamic_cast<GameCreature*>(JE::GetEntityPtr(entityID));
		if (weapon)
		{
			if (weapon->currentAttackMode == AttackModeIterator())
			{
				if (entityID)
				{
					FeedbackInterface* feedback = client->Feedback();
					feedback->AddMessage(std::string("Reload!"),0,128,0,std::string());
					client->RequestReload(entityID,weapon->entityID,0);
					SetAttackMode(index,1);
				}
			}
			else SetCursorContext(client,15);
		}
		else if (item) SetCursorContext(client,16);
		else if (creature && creature->CurrentAttackMode()) SetCursorContext(client,15);
	}
	void ActionbarInterface::SetEvasionAP(DWORD index) //4186F0, only diagnostic feedback.
	{
		const LONG point=static_cast<LONG>(index);
		if (dynamic_cast<GameCreature*>(JE::GetEntityPtr(entityID)) && point>=0 && point<30)
		{
			const LONG displayed=point+1;
			GameClient* client=dynamic_cast<GameClient*>(static_cast<JE::Client*>(g_pNetworkInstance->client));
			FeedbackInterface* feedback=client->Interfaces()->feedback;
			char text[256]; std::sprintf(text,"Wanting to set evasion AP to %d",displayed);
			feedback->AddMessage(std::string(text),128,128,0,std::string(""));
		}
	}
	int ActionbarInterface::OnEvent(::GUI::Window* window,::GUI::Event* event) //41CA00/41CFE0.
	{
		if (!GetHidden())
		{
			switch (event->type)
			{
			case 0:
				if (window==rootWindow) Tick();
				break;
			case 6:
				if (window==sneakButton)
				{
					std::string text;
					if (sneaking) { text="You stop sneaking."; sneaking=0; }
					else { text="You begin to sneak."; sneaking=1; }
					ActionbarServices::Feedback(text,0,128,0);
				}
				else if (window==inventoryButton) ActionbarServices::QueueCommand("InventoryWindow");
				else if (window==pipBoyButton) ActionbarServices::QueueCommand("PipBoy");
				else if (window==useSkillButton) SetCursorContext(16);
				else if (window==characterButton) {} //Original empty button branch.
				else if (window==optionsButton) ActionbarServices::QueueCommand("OptionsWindow");
				else if (window==weaponButtons[0]) { if (!otherEntityID) PrimaryWeaponClick(0); }
				else if (window==weaponButtons[1]) { if (!otherEntityID) PrimaryWeaponClick(1); }
				else if (window==actionPointsLabel && dynamic_cast<GameCreature*>(JE::GetEntityPtr(entityID)))
					ActionbarServices::Feedback("Wanting to set evasion AP to 0",128,128,0);
				break;
			case 4:
				for (LONG i=0;i<6;++i) if (window==partyPictures[i]) SetSelectedPartyMember(i);
				for (DWORD i=0;i<2;++i)
					if (window==weaponButtons[i])
					{
						if (entityID)
						{
							GameClient* client=dynamic_cast<GameClient*>(g_pNetworkInstance->client);
							client->RequestWeaponSelection(entityID,i+2);
						}
						otherEntityID=!IsWeaponActive(i);
						SetWeaponActive(i,1);
						SetWeaponActive(1-i,0);
					}
				break;
			case 14:
				for (DWORD i=0;i<2;++i)
					if (window==weaponButtons[i])
					{
						if (popups[0]->button->GetHidden() && IsWeaponActive(i)) SetAttackMode(i,1);
						ClearStatusButtons();
					}
				for (DWORD i=0;i<30;++i) if (window==actionPointIcons[i]) SetEvasionAP(i);
				break;
			case 12:
			case 15:
				for (DWORD i=0;i<2;++i)
					if (window==weaponButtons[i] && IsWeaponActive(i))
					{
						const DWORD time=timeGetTime();
						popupValue294=i;
						popupValue290=time;
					}
				break;
			case 13:
				for (DWORD i=0;i<6;++i)
					if (window==popups[i]->button)
					{
						if (IsWeaponActive(0)) SetAttackMode(0,i+1);
						else if (IsWeaponActive(1)) SetAttackMode(1,i+1);
					}
				if (window!=weaponButtons[0] && window!=weaponButtons[1]) ClearStatusButtons();
				break;
			}
		}
		return JE::EngineInterface::OnEvent(window,event);
	}
}
