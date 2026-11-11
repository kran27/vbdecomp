// Original Interface FindAt4952B0/87; ordinary roots and unchanged six-slot class.
#include "gui/Interface.hpp"
#include "gui/Window.hpp"

namespace GUI
{
	Window* Interface::FindAt(LONG x, LONG y, BYTE overrideSelf)
	{
		if (!hidden)
		{
			for (std::list<Window*>::iterator entry = roots.begin(); entry != roots.end(); ++entry)
			{
				Window* result = (*entry)->FindAt(x, y, overrideSelf);
				if (result)
					return result;
			}
		}
		return NULL;
	}
}
