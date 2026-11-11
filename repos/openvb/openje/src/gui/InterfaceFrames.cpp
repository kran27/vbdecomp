#include "Interface.hpp"
#include "Window.hpp"

namespace GUI
{
	void Interface::RefreshPositions()
	{
		for (std::list<Window*>::iterator entry = roots.begin(); entry != roots.end(); ++entry)
			(*entry)->Offset(0, 0);
	}

	void Interface::Update()
	{
		if (!hidden)
		{
			BeforeUpdate();
			std::list<Window*>::iterator entry = roots.end();
			--entry;
			while (entry != roots.end())
			{
				(*entry)->Refresh();
				--entry;
			}
		}
	}

	void Interface::Draw()
	{
		if (!hidden)
		{
			BeforeDraw();
			std::list<Window*>::iterator entry = roots.end();
			--entry;
			while (entry != roots.end())
			{
				(*entry)->Draw();
				--entry;
			}
		}
	}
}
