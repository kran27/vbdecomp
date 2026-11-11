#include "Manager.hpp"

namespace GUI
{
	void Manager::MarkDepthDirty()
	{
		depthDirty = 1;
	}

	BOOL Manager::HasModal()
	{
		return !modal.empty();
	}

	void Manager::RemoveModal(Interface* value)
	{
		modal.remove(value);
	}

	Interface* Manager::FrontModal()
	{
		if (modal.empty())
			return NULL;
		return modal.front();
	}

	void Manager::BringToFront(Interface* value)
	{
		for (std::list<Interface*>::iterator entry = interfaces.begin();
			entry != interfaces.end(); ++entry)
		{
			if (value == *entry)
			{
				interfaces.erase(entry);
				interfaces.push_front(value);
				MarkDepthDirty();
				return;
			}
		}
	}
}
