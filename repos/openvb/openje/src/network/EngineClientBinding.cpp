#include "EngineClientBinding.hpp"
#include <map>

namespace JE
{
	namespace
	{
		typedef std::map<const Client*, EngineClientBinding> Bindings;
		Bindings* bindings = NULL;
	}

	EngineClientBinding::EngineClientBinding()
		: unconsumedPollingFrame(NULL), unconsumedPollingFrameContext(NULL),
		unconsumedInterfaceAction(NULL), unconsumedInterfaceActionContext(NULL),
		unconsumedDialogueUpdate(NULL), unconsumedDialogueUpdateContext(NULL),
		borrowedLightingState(NULL), borrowedGraphicsState(NULL), borrowedInputState(NULL),
		borrowedInputServices(NULL), borrowedGraphicsInput(NULL)
	{
	}

	EngineClientBinding& GetEngineClientBinding(const Client& client)
	{
		if (!bindings) bindings = new Bindings;
		return (*bindings)[&client];
	}
	EngineClientBinding* FindEngineClientBinding(const Client& client)
	{
		if (!bindings) return NULL;
		Bindings::iterator found = bindings->find(&client);
		return found == bindings->end() ? NULL : &found->second;
	}

	void ClearEngineClientBinding(const Client& client)
	{
		if (!bindings) return;
		bindings->erase(&client);
		if (bindings->empty())
		{
			delete bindings;
			bindings = NULL;
		}
	}
}
