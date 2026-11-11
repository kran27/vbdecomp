#include "GraphicsPresentation.hpp"
#include "SystemInterface.hpp"
#include "gui/Manager.hpp"
#include <stdexcept>

namespace JE
{
	void PresentGraphicsFrame(DWORD& previousClock, const BYTE& clientFlag10A,
		::GUI::Manager* manager, Gfx_System_Interface*& backend, DWORD (*clock)())
	{
		if (!clientFlag10A)
		{
			if (!manager) throw std::logic_error("Graphics5811DC: actual GUI Manager required");
			manager->Render();
		}
		const DWORD now = clock();
		backend->AdvanceTime(now - previousClock);
		backend->RenderFrame();
		previousClock = now;
	}
}
