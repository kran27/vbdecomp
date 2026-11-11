// Original GUI input family: 4B3780,4B3920,4B40A0,4B40F0.
#include "gui/Manager.hpp"
#include "gui/Interface.hpp"
#include "gui/Event.hpp"

#include <mbstring.h>

namespace GUI
{
	int Manager::DispatchInterfaceEvent(Event* event, Interface* owner)
	{
		std::list<Window*>* roots = owner->GetRoots();
		std::list<Window*>::iterator entry = roots->begin();
		while (entry != roots->end())
		{
			if (DispatchTree(event, owner, *entry) == 1)
			{
				return 1;
			}
			++entry;
		}
		return 0;
	}

	int Manager::DispatchInputEvent(Event* event)
	{
		std::list<Interface*>::iterator entry = modal.begin();
		while (entry != modal.end())
		{
			Interface* current = *entry;
			if (!current->GetHidden())
			{
				DispatchInterfaceEvent(event, current);
				return 1;
			}
			++entry;
		}

		entry = interfaces.begin();
		while (entry != interfaces.end())
		{
			Interface* current = *entry;
			if (!current->GetHidden()
				&& DispatchInterfaceEvent(event, current) == 1)
			{
				return 1;
			}
			++entry;
		}
		return 0;
	}

	BYTE Manager::ProcessKey(LONG key, BYTE down, DWORD modifiers)
	{
		BYTE handled = 0;
		if (inputBlocked)
		{
			return 0;
		}
		// Only these three DWORDs are written by the original producer.
		Event event;
		event.type = down ? 17 : 18;
		event.x = key;
		event.modifiers = modifiers;
		if (DispatchInputEvent(&event) == 1) handled = 1;
		return handled;
	}

	BYTE Manager::ProcessCharacter(char character, DWORD modifiers)
	{
		BYTE handled = 0;
		if (inputBlocked)
		{
			return 0;
		}
		// MOVSX in 4B4110 preserves signed-char promotion into the real CRT API.
		if (_ismbcgraph(static_cast<unsigned int>(static_cast<int>(character))) || character == ' ')
		{
		Event event;
		event.modifiers = modifiers;
		event.type = 19;
		// Original 4B413C writes one BYTE; the other bytes of x stay unwritten.
		*reinterpret_cast<BYTE*>(&event.x) = static_cast<BYTE>(character);
		if (DispatchInputEvent(&event) == 1) handled = 1;
		}
		return handled;
	}
}
