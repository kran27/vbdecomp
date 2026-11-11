#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/ActionbarInterface.hpp"
#include "game/GameEntity.hpp"

namespace F3
{
	void GameClient::ToggleActionbarWindow() //45EA50.
	{
		GameClient* client=static_cast<GameClient*>(g_pNetworkInstance->client);
		ActionbarInterface* actionbar=client->Interfaces()->actionbar; //Native unchecked owner, captured across callbacks.
		if (actionbar->GetHidden())
		{
			const DWORD id=static_cast<JE::Client*>(g_pNetworkInstance->client)->field0xE0;
			actionbar->SetHidden(0);
			actionbar->RefreshEntity(id);
		}
		else
		{
			actionbar->SetHidden(1);
			actionbar->HideCompanions();
		}
	}
	void GameClient::ToggleWeapon() //45EED0; actual selected entity, then bound UI controls.
	{
		GameClient* client=static_cast<GameClient*>(g_pNetworkInstance->client);
		GameInterfaces* interfaces=client->Interfaces();
		const DWORD selected=client->field0xE0;
		ActionbarInterface* actionbar=interfaces->actionbar;
		GameEntity* entity=dynamic_cast<GameEntity*>(JE::GetEntityPtr(selected));
		if (!entity) return;
		switch (entity->GetActiveWeaponSlot())
		{
		case 2:
			entity->SetActiveWeaponSlot(3);
			actionbar->SetWeaponActive(0,0);
			actionbar->SetWeaponActive(1,1);
			break;
		case 3:
			entity->SetActiveWeaponSlot(2);
			actionbar->SetWeaponActive(0,1);
			actionbar->SetWeaponActive(1,0);
			break;
		}
	}
	void GameClient::CycleAttackMode() //45EE60.
	{
		GameClient* client=static_cast<GameClient*>(g_pNetworkInstance->client);
		GameInterfaces* interfaces=client->Interfaces();
		const DWORD selected=client->field0xE0;
		ActionbarInterface* actionbar=interfaces->actionbar;
		GameEntity* entity=dynamic_cast<GameEntity*>(JE::GetEntityPtr(selected));
		if (!entity) return;
		switch (entity->GetActiveWeaponSlot())
		{
		case 2: actionbar->SetAttackMode(0,1); break;
		case 3: actionbar->SetAttackMode(1,1); break;
		}
	}
}
