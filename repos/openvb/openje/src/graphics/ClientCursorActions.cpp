#include <winsock2.h>
#include "ClientCursor.hpp"
#include "FloatingTextInterface.hpp"
#include "entity/Entity.hpp"
#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/GameCreature.hpp"
#include "game/AttackMode.hpp"
#include "game/GameInterfaces.hpp"
#include "game/FeedbackInterface.hpp"
#include "game/GameSkillMenu.hpp"
#include "game/ClientUseRequests.hpp"
#include "game/ClientHostileActions.hpp"
#include "game/ClientDialogueActions.hpp"
#include "game/ClientDoorRequests.hpp"
#include "game/ClientPickupRequests.hpp"
#include "game/ClientAttackRequests.hpp"
#include <stdexcept>

namespace F3
{
	BYTE GameCursorManager::LeftRelease(LONG x, LONG y) //436B40.
	{
		//436B60 captures the published receiver before either Entity lookup.
		//Callbacks may replace the published Client; all requests below keep this one.
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		const DWORD selectedID = client->field0xE0;
		const DWORD hoveredID = client->field0xDC;
		JE::Entity* selected = JE::GetEntityPtr(selectedID);
		JE::Entity* hovered = JE::GetEntityPtr(hoveredID);
		const DWORD action = context == 1 ? subcontext : context;
		BYTE handled = 1;
		switch (action)
		{
		case 3:
			if (selected && hovered) client->RequestUse(selectedID,hoveredID);
			break;
		case 4:
			if (selected && hovered)
			{
				client->RequestTalk(selectedID,hoveredID);
				if (selected == hovered)
				{
					std::string text("You talk to yourself, but recieve no answer.");
					client->child0xAC->AddSpeech(selectedID,text,2000,128,128,128); //587A60 forwarding.
				}
			}
			break;
		case 5:
			if (selected && hovered) client->RequestHostile(selectedID,hoveredID);
			break;
		case 6:
			if (selected && hovered) client->RequestContext75(selectedID,hoveredID);
			break;
		case 7:
			if (selected && hovered) client->RequestContext76(selectedID,hoveredID);
			break;
		case 8:
			if (selected && hovered) client->RequestContext77(selectedID,hoveredID);
			break;
		case 9:
			if (selected && hovered) client->RequestOperate(selectedID,hoveredID);
			break;
		case 10:
			if (selected && hovered) client->RequestContext79(selectedID,hoveredID);
			break;
		case 11:
			if (selected && hovered) client->RequestContext80(selectedID,hoveredID);
			break;
		case 12:
			if (selected && hovered) client->RequestLook(selectedID,hoveredID);
			break;
		case 13:
			if (selected && hovered) client->RequestPickup(selectedID,hoveredID);
			break;
		case 14:
			if (selected && hovered)
			{
				//41D1F0 reads this+38: the dragged item, not a map/row index.
				const DWORD itemID = draggedItemID;
				client->RequestGive(selectedID,hoveredID,itemID);
			}
			break;
		case 15:
		{
			GameCreature* creature = dynamic_cast<GameCreature*>(selected);
			if (creature)
			{
				GameWeapon* weapon = NULL;
				AttackModeIterator mode;
				BYTE unarmed;
				if (creature->ResolveWeaponAndAttackMode(&weapon,&mode,&unarmed))
				{
					if (hovered)
					{
						if (selected == hovered)
						{
							std::string text("You start to attack yourself, but think better of it.");
							client->child0xAC->AddSpeech(selectedID,text,3500,128,128,128);
						}
						else
						{
							FeedbackInterface* feedback = static_cast<GameInterfaces*>(client->child0xA0)->feedback;
							feedback->AddMessage("Targeted attack on entity.",0,128,0,"");
							client->RequestAttack(selected->entityID,hovered->entityID,9,NULL);
						}
					}
					else if (mode->flag65)
					{
						D3DXVECTOR3 point(0,0,0);
						if (client->TryGetGroundPosition(point)) //587AC0, live late read.
						{
							FeedbackInterface* feedback = static_cast<GameInterfaces*>(client->child0xA0)->feedback;
							feedback->AddMessage("Targeted attack on floor.",0,128,0,"");
							client->RequestAttack(selected->entityID,0,9,&point);
						}
					}
				}
			}
			break;
		}
		case 16:
			if (selected && hovered) client->RequestSkillMenu(selectedID,hoveredID,x,y);
			break;
		default:
			handled = 0;
			break;
		}
		SetContext(1); //Actual436AD0 invokes the original no-argument UpdateFrame.
		return handled;
	}

}
