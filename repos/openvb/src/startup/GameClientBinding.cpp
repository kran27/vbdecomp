#include "GameClientBinding.hpp"
#include <map>

namespace F3
{
	namespace
	{
		typedef std::map<const GameClient*, GameClientBinding> Bindings;
		Bindings* bindings = NULL;
	}

	GameClientBinding::GameClientBinding()
		: unconsumedInventoryInspection(NULL), unconsumedInventoryInspectionContext(NULL),
		borrowedWorldContainer(NULL), borrowedInventory(NULL), borrowedActionbar(NULL),
		borrowedFeedback(NULL), borrowedMiniMap(NULL), borrowedEscapeMenu(NULL),
		borrowedOptions(NULL), borrowedPipBoy(NULL), inventoryInspectionObserver(NULL),
		inventoryInspectionObserverContext(NULL)
	{
	}

	GameClientBinding& GetGameClientBinding(const GameClient& client)
	{
		if (!bindings) bindings = new Bindings;
		return (*bindings)[&client];
	}

	const GameClientBinding* FindGameClientBinding(const GameClient& client)
	{
		if (!bindings) return NULL;
		const Bindings::const_iterator found = bindings->find(&client);
		return found == bindings->end() ? NULL : &found->second;
	}

	void ClearGameClientBinding(const GameClient& client)
	{
		if (!bindings) return;
		bindings->erase(&client);
		if (bindings->empty())
		{
			delete bindings;
			bindings = NULL;
		}
	}

#define F3_BORROWED_INTERFACE(type, method, member, borrowed) \
	type* GameClient::method() const \
	{ \
		if (Interfaces()) return Interfaces()->member; \
		const GameClientBinding* binding = FindGameClientBinding(*this); \
		return binding ? binding->borrowed : NULL; \
	}
	F3_BORROWED_INTERFACE(ActionbarInterface, Actionbar, actionbar, borrowedActionbar)
	F3_BORROWED_INTERFACE(FeedbackInterface, Feedback, feedback, borrowedFeedback)
	F3_BORROWED_INTERFACE(InventoryInterface, Inventory, inventory, borrowedInventory)
	F3_BORROWED_INTERFACE(MiniMapInterface, MiniMap, miniMap, borrowedMiniMap)
	F3_BORROWED_INTERFACE(EscapeMenuInterface, EscapeMenu, escapeMenu, borrowedEscapeMenu)
	F3_BORROWED_INTERFACE(OptionsInterface, Options, options, borrowedOptions)
	F3_BORROWED_INTERFACE(PipBoyInterface, PipBoy, pipBoy, borrowedPipBoy)
	F3_BORROWED_INTERFACE(JE::WorldContainerInterface, WorldContainer, container, borrowedWorldContainer)
#undef F3_BORROWED_INTERFACE
}
