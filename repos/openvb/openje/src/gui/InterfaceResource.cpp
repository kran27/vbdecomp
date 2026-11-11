#include "Interface.hpp"
#include "Window.hpp"
#include "WindowFactory.hpp"
#include "ResourceIO.hpp"

namespace GUI
{
	void Interface::ClearWindows()
	{
		roots.clear(); // 4955B0; does not delete or detach old roots.
	}

	BYTE Interface::LoadResource(const std::string& filename)
	{
		ClearWindows();
		JE::Resources::ResourceFileHandle* file = ResourceIO::Open(filename.c_str());
		if (!file) return 0;
		int version = 0;
		DWORD head; // Original ignores header-read status and leaves bytes unwritten.
		ResourceIO::Read(&head, 4, file);
		if (head == 0x20495547)
		{
			DWORD text;
			ResourceIO::Read(&text, 4, file);
			switch (text)
			{
			case 0x302E3156: version = 1; break;
			case 0x312E3156: version = 2; break;
			case 0x322E3156: version = 3; break;
			case 0x332E3156: version = 4; break;
			}
		}
		else ResourceIO::Seek(file, 0, 0);
		DWORD count;
		if (!ReadResourceString(name, file)
			|| !ResourceIO::Read(&hidden, 1, file)
			|| !ResourceIO::Read(&count, 4, file))
		{
			ResourceIO::Release(file);
			return 0;
		}
		for (DWORD i = 0; i < count; ++i)
		{
			DWORD type;
			if (!ResourceIO::Read(&type, 4, file))
			{
				ResourceIO::Release(file);
				return 0;
			}
			Window* window = g_Manager.CreateWindowByType(type);
			if (!window->ReadResource(file, version))
			{
				ResourceIO::Release(file);
				return 0; // Original failed, unpublished Window is not deleted.
			}
			AddWindow(window);
		}
		roots.reverse();
		ResourceIO::Release(file);
		std::list<std::list<Window*>*> pending;
		pending.push_back(GetRoots());
		while (!pending.empty())
		{
			std::list<Window*>* windows = pending.front();
			pending.pop_front();
			for (std::list<Window*>::iterator entry = windows->begin(); entry != windows->end(); ++entry)
			{
				Window* window = *entry;
				pending.push_back(window->GetChildren());
				if (window->type == 1)
				{
					std::string skin;
					window->GetSkin(skin);
					window->ApplySkin(skin);
				}
			}
		}
		return 1;
	}
}
