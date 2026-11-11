// Original base Window event predicate4AFFA0/83. No event handler is invoked.
#include "gui/Window.hpp"
#include "gui/Event.hpp"

namespace GUI
{
	BOOL Window::OnEvent(Event* event)
	{
		// Preserve the observed signed upper-bound check and absence of a
		// lower-bound check. Negative indices are outside the valid C++ domain.
		return event->type < static_cast<int>(strings.size()) &&
			!strings[event->type].empty();
	}
}
