#include <winsock2.h>
#include "ClientCursor.hpp"
#include "ClientCursorActions.hpp"
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
#include <map>

namespace
{
	typedef std::map<F3::GameCursorManager*, F3::GameCursorActionServices*> CursorHostBindings;
	CursorHostBindings& HostBindings()
	{
		static CursorHostBindings bindings;
		return bindings;
	}
}

namespace F3
{
	void GameCursorActionServices::Feedback(const char*)
	{ throw std::logic_error("GameCursorManager434B70 requires actual Client feedback text owner"); }
	void GameCursorActionServices::SelfSpeech(DWORD, const char*, DWORD, BYTE, BYTE, BYTE)
	{ throw std::logic_error("GameCursorManager587A60 requires actual floating speech request owner"); }
	void GameCursorActionServices::SendAttack(DWORD, DWORD, DWORD, const D3DXVECTOR3*)
	{ throw std::logic_error("GameCursorManager45F370 requires actual Client attack transport"); }
	void GameCursorActionServices::SendPickup(DWORD, DWORD)
	{ throw std::logic_error("GameCursorManager45FDB0 requires actual Client pickup transport"); }
	void GameCursorActionServices::SendOperate(DWORD, DWORD)
	{ throw std::logic_error("GameCursorManager45FA30 requires actual Client operate transport"); }
	void GameCursorActionServices::SendUse(DWORD, DWORD)
	{ throw std::logic_error("GameCursorManager45F4F0 requires actual Client use transport"); }
	void GameCursorActionServices::SendTalk(DWORD, DWORD)
	{ throw std::logic_error("GameCursorManager45F5D0 requires actual Client talk transport"); }
	void GameCursorActionServices::SendHostile(DWORD, DWORD)
	{ throw std::logic_error("GameCursorManager45F6B0 requires actual Client hostile-context transport"); }
	void GameCursorActionServices::OpenContextMenu(DWORD, DWORD, LONG, LONG)
	{ throw std::logic_error("GameCursorManager587A80 requires actual ContextMenu568970 request owner"); }
	void GameCursorActionServices::OpenSkillMenu(DWORD, DWORD, LONG, LONG)
	{ throw std::logic_error("GameCursorManager45EF70 requires actual GameCSMenuInterface skill owner"); }
	void GameCursorActionServices::Tooltip(DWORD)
	{ throw std::logic_error("GameCursorManager436840 requires actual tooltip Window/Label owner"); }

	GameCursorActionServices& GameCursorManager::Actions()
	{
		static GameCursorActionServices missing;
		CursorHostBindings& bindings = HostBindings();
		CursorHostBindings::iterator found = bindings.find(this);
		return found != bindings.end() ? *found->second : missing;
	}
	void GameCursorManager::BindActionServices(GameCursorActionServices* services)
	{
		if (services) HostBindings()[this] = services;
		else HostBindings().erase(this);
	}

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

	BYTE GameCursorManager::LeftRelease(JE::ClientInputState& input, LONG x, LONG y, HWND window)
	{
		const DWORD selectedID = input.selectedEntityID, hoveredID = input.hoveredEntityID;
		JE::Entity* selected = JE::GetEntityPtr(selectedID);
		JE::Entity* hovered = JE::GetEntityPtr(hoveredID);
		const DWORD action = context == 1 ? subcontext : context;
		BYTE handled = 1;
		if (action >= 3 && action <= 16)
		{
			if (action == 15)
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
								Actions().SelfSpeech(selectedID,
									"You start to attack yourself, but think better of it.",3500,128,128,128);
							else
							{
								Actions().Feedback("Targeted attack on entity.");
								Actions().SendAttack(selected->entityID,hovered->entityID,9,NULL);
							}
						}
						else if (mode->flag65)
						{
							D3DXVECTOR3 point(0,0,0);
							if (input.groundHit)
							{
								point = input.groundPosition; //587AC0's actual copy only on a hit.
								Actions().Feedback("Targeted attack on floor.");
								Actions().SendAttack(selected->entityID,0,9,&point);
							}
						}
					}
				}
			}
			else if (action == 16)
			{
				if (selected && hovered) Actions().OpenSkillMenu(selectedID,hoveredID,x,y);
			}
			else if (action == 3)
			{
				if (selected && hovered) Actions().SendUse(selectedID,hoveredID);
			}
			else if (action == 4)
			{
				if (selected && hovered)
				{
					Actions().SendTalk(selectedID,hoveredID);
					if (selected == hovered)
						Actions().SelfSpeech(selectedID,
							"You talk to yourself, but recieve no answer.",2000,128,128,128);
				}
			}
			else if (action == 5)
			{
				if (selected && hovered) Actions().SendHostile(selectedID,hoveredID);
			}
			else if (action == 9)
			{
				if (selected && hovered) Actions().SendOperate(selectedID,hoveredID);
			}
			else if (action == 13)
			{
				if (selected && hovered) Actions().SendPickup(selectedID,hoveredID);
			}
			else if (selected && hovered)
				throw std::logic_error("GameCursorManager436BAB populated context action3..16 required");
		}
		else handled = 0;
		SetContext(1,input,window);
		return handled;
	}
}
