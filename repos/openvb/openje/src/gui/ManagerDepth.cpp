

#include "gui/Manager.hpp"
#include "gui/Interface.hpp"
#include "gui/Window.hpp"

namespace GUI
{
	// 0x4B3570. Hidden interfaces contribute to the wrapped DWORD total.
	DWORD Manager::CountDepthSlots()
	{
		DWORD count = 0;
		for (std::list<Interface*>::iterator entry = interfaces.begin(); entry != interfaces.end(); ++entry)
		{
			std::list<Window*>* roots = (*entry)->GetRoots();
			for (std::list<Window*>::iterator root = roots->begin(); root != roots->end(); ++root)
				count += CountWindowDepthSlots(*root);
		}
		return count;
	}

	// 0x4B2F80. A normal childless Window contributes1 + its own returned1.
	DWORD Manager::CountWindowDepthSlots(Window* window)
	{
		DWORD count = 0;
		std::list<Window*>* children = window->GetChildren();
		for (std::list<Window*>::iterator entry = children->begin(); entry != children->end(); ++entry)
			count += CountWindowDepthSlots(*entry);
		return count + window->GetDepthSlots() + 1;
	}
}
