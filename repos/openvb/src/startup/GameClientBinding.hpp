#pragma once

#include "GameClient.hpp"

namespace F3
{
	//External workspace bookkeeping. All pointers are borrowed and none are
	//members of the original GameClient. The explicit host owns this lifetime.
	struct GameClientBinding
	{
		GameClientBinding();
		GameClient::UnconsumedInventoryInspection unconsumedInventoryInspection;
		void* unconsumedInventoryInspectionContext;
		JE::WorldContainerInterface* borrowedWorldContainer;
		InventoryInterface* borrowedInventory;
		ActionbarInterface* borrowedActionbar;
		FeedbackInterface* borrowedFeedback;
		MiniMapInterface* borrowedMiniMap;
		EscapeMenuInterface* borrowedEscapeMenu;
		OptionsInterface* borrowedOptions;
		PipBoyInterface* borrowedPipBoy;
		GameClient::InventoryInspectionObserver inventoryInspectionObserver;
		void* inventoryInspectionObserverContext;
	};

	GameClientBinding& GetGameClientBinding(const GameClient& client);
	const GameClientBinding* FindGameClientBinding(const GameClient& client);
	void ClearGameClientBinding(const GameClient& client);
}
