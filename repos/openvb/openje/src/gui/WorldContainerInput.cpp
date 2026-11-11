#include "network/EngineClient.hpp"
#include "gui/WorldContainerInterface.hpp"
#include "gui/Window.hpp"
#include "gui/Event.hpp"
#include "gui/ScrollBar.hpp"
#include "game/GameItem.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/World.hpp"
#include <cstring>

namespace JE
{
	int WorldContainerInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event) //4575D0.
	{
		if (scrollbar && scrollbar->Process(window, event) == 1) RefreshRows();
		if (event->type == 6)
		{
			Client& client = *static_cast<Client*>(F3::g_pNetworkInstance->client); //Capture before GetName,457606..45760F.
			if (!::_stricmp(window->GetName(), "Done"))
			{
				SetHidden(1);
				clickSound.Play(0, 0, 0); //OriginalCSound+40 call4C7EB0.
			}
			else if (!::_stricmp(window->GetName(), "TakeAll"))
			{
				clickSound.Play(0, 0, 0);
				//Live vector/end and per-item owner fields, not an inventory snapshot.
				for (std::vector<F3::GameItem*>::iterator i = items.begin(); i != items.end(); ++i)
				{
					DWORD sourceID = targetID;
					DWORD itemID = (*i)->entityID;
					DWORD destinationID = actorID;
					client.RequestInventoryTransfer(destinationID, itemID, sourceID);
				}
			}
			else
			{
				F3::GameItem* item = FindRowItem(window);
				if (item)
				{
					clickSound.Play(0, 0, 0);
					DWORD sourceID = targetID;
					DWORD itemID = item->entityID;
					DWORD destinationID = actorID;
					client.RequestInventoryTransfer(destinationID, itemID, sourceID);
				}
			}
		}
		if (event->type == 0 && window == GetRoots()->front())
		{
			//The original assumes its constructor-populated root and actual World.
			(void)GetWorldInstance()->GetTime(); //Original IWorld slot0; result discarded.
			Entity* actor = GetEntityPtr(actorID);
			if (!actor || actor->moving == 1 || !actor->renderEnabled
				|| actor->IsDead() == 1 || !actor->IsVisible()) SetHidden(1);
			//Do not early-return after hiding actor failure. Target is still resolved.
			Entity* target = GetEntityPtr(targetID);
			if (!target || target->moving == 1 || !target->renderEnabled || !target->IsVisible())
				SetHidden(1); //There is deliberately no IsDead test on the container/corpse.
		}
		return ::GUI::Interface::OnEvent(window, event); //Actualdirect4958D0, not EngineInterface.
	}
}
