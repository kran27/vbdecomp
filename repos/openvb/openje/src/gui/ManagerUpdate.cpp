// Original-backed candidate: 4B2F30/79,4B3500/112,4B37F0/290.
// All three related operations remain together; real VC7 supplies list APIs.
#include "gui/Manager.hpp"
#include "gui/Interface.hpp"
#include "gui/Window.hpp"
#include "gui/Event.hpp"

namespace GUI
{
	int Manager::DispatchEvent(Event* event, Interface* owner, Window* window)
	{
		if (!owner)
		{
			return 0;
		}
		if (!modal.empty() && owner != modal.front())
		{
			return 0;
		}
		int result = owner->OnEvent(window, event);
		if (!result)
		{
			result = window->OnEvent(event);
		}
		return result;
	}

	int Manager::DispatchTree(Event* event, Interface* owner, Window* window)
	{
		int result = DispatchEvent(event, owner, window);
		if (!result)
		{
			std::list<Window*>* children = window->GetChildren();
			std::list<Window*>::iterator entry = children->begin();
			while (entry != children->end())
			{
				result = DispatchTree(event, owner, *entry);
				if (result == 1)
				{
					break;
				}
				++entry;
			}
		}
		return result;
	}

	void Manager::Update()
	{
		std::list<Interface*>::iterator entry = interfaces.begin();
		while (entry != interfaces.end())
		{
			Interface* current = *entry;
			if (current->pendingDestroy)
			{
				entry = interfaces.erase(entry);
				MarkDepthDirty();
				if (hoveredOwner == current)
				{
					hoveredOwner = NULL;
				}
				delete current;
			}
			else
			{
				++entry;
			}
		}

		// The original writes only the first DWORD. Callbacks share all six
		// words throughout this pass; the remaining initial words are unknown.
		Event event;
		event.type = 0;
		entry = interfaces.begin();
		while (entry != interfaces.end())
		{
			Interface* current = *entry;
			if (!current->GetHidden())
			{
				std::list<Window*>* roots = current->GetRoots();
				std::list<Window*>::iterator root = roots->begin();
				while (root != roots->end())
				{
					if (DispatchTree(&event, current, *root) == 1)
					{
						break;
					}
					++root;
				}
			}
			++entry;
		}

		// Retain the current node across callbacks, as in the recovered Render.
		entry = interfaces.end();
		--entry;
		while (entry != interfaces.end())
		{
			Interface* current = *entry;
			if (!current->GetHidden())
			{
				current->Update();
			}
			--entry;
		}
		AssignDepth();
	}
}
