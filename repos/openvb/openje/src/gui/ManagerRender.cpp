// GUI::Manager rendering at original 0x4B3690 (227 bytes).
// Preserve borrowed viewport routing and held-node interface traversal.
#include "gui/Manager.hpp"
#include "gui/Interface.hpp"
#include "gui/PacketManager.hpp"
#include "gui/PacketStorage.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Viewport.hpp"
#include "graphics/RenderManager.hpp"

namespace GUI
{
	void Manager::AddViewport(Gfx_Viewport* viewport)
	{
		entries148.push_back(viewport);
	}

	void Manager::Render()
	{
		packets->Clear();
		storage->Reset();
		if (view)
		{
			g_GfxSystem->AddViewport(view);
			Gfx_Viewport* currentView = view;
			Gfx_MeshInstance* currentInstance = &currentView->instance;
			Gfx_RenderManager* currentRenderer = currentView->renderer;
			currentView->flag1C = 1;
			currentRenderer->QueueMeshInstances(currentInstance, 1);
		}

		std::list<Gfx_Viewport*>::iterator viewport = entries148.begin();
		while (viewport != entries148.end())
		{
			if (*viewport)
			{
				g_GfxSystem->AddViewport(*viewport);
			}
			++viewport;
		}
		entries148.clear();

		// Keep the currently held node across the callback. An ordinary
		// reverse_iterator instead retains its one-ahead base and can revisit
		// an appended entry. Only valid current-node-preserving lists apply.
		std::list<Interface*>::iterator entry = interfaces.end();
		--entry;
		while (entry != interfaces.end())
		{
			Interface* current = *entry;
			if (!current->GetHidden())
			{
				current->Draw();
			}
			--entry;
		}
		packets->Flush();
	}
}
