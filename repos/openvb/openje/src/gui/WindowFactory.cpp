#include "WindowFactory.hpp"
#include "Window.hpp"
#include "Button.hpp"
#include "Picture.hpp"
#include "Label.hpp"
#include "Edit.hpp"
#include <stdexcept>

namespace GUI
{
	Window* Manager::CreateWindowByType(DWORD type)
	{
		switch (type)
		{
		case 0: return new Window(0);
		case 1: return new Button(1);
		case 2: return new Picture(2);
		case 3: return new Label(3); //Original4B2A09 passes type3 to the actual Label constructor.
		case 4: return new Edit(4); //4C51B0; editing/render frontiers remain explicit.
		default: return NULL;
		}
		// Normal reused C++ allocation/EH; original NULL-new branches unvalidated.
	}
}
