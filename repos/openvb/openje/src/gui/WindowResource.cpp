#include "Window.hpp"
#include "WindowFactory.hpp"
#include "ResourceIO.hpp"

namespace GUI
{
	void Window::ClearChildren()
	{
		// 4B0000 clears pointer-list nodes only; old children are not deleted.
		children.clear();
	}

	BYTE Window::ReadResource(JE::Resources::ResourceFileHandle* file, int version)
	{
		ClearChildren();
		if (!Base::ReadResource(file, version)
			|| !ReadResourceString(name, file)
			|| !ResourceIO::Read(&hidden, 1, file)
			|| !ResourceIO::Read(&frameVisible, 1, file)
			|| !ResourceIO::Read(&centerVisible, 1, file)
			|| !ReadResourceString(skin, file)
			|| !ResourceIO::Read(&color, 4, file)) return 0;
		if (version < 2 && static_cast<BYTE>(color >> 24) != 255)
		{
			materialModes = 1;
		}
		if (!ResourceIO::Read(&clientRectangle, 16, file)) return 0;
		if (version >= 2)
		{
			if (!ResourceIO::Read(&value60, 1, file)
				|| !ResourceIO::Read(&materialModes, 4, file)) return 0;
			SetMaterialModes(materialModes);
		}
		if (version >= 3)
		{
			if (!ReadResourceString(stringCB8, file)
				|| !ResourceIO::Read(&valueCD4, 4, file)
				|| !ReadResourceString(stringCD8, file)
				|| !ResourceIO::Read(&valueCF4, 4, file)
				|| !ReadResourceString(stringCF8, file)) return 0;
		}
		for (int i = 0; i < 9; ++i)
		{
			if (!fragments[i].ReadResource(file, version)) return 0;
		}
		strings.clear();
		DWORD index;
		if (!ResourceIO::Read(&index, 4, file)) return 0;
		while (index != 0xFFFFFFFF)
		{
			if (index >= strings.size()) strings.resize(static_cast<DWORD>(index + 5));
			if (!ReadResourceString(strings[index], file)
				|| !ResourceIO::Read(&index, 4, file)) return 0;
		}
		owner = NULL;
		parent = NULL;
		DWORD count;
		if (!ResourceIO::Read(&count, 4, file)) return 0;
		for (DWORD childIndex = 0; childIndex < count; ++childIndex)
		{
			DWORD type;
			if (!ResourceIO::Read(&type, 4, file)) return 0;
			Window* child = g_Manager.CreateWindowByType(type);
			// The original dereferences NULL and leaks failed unpublished children.
			if (!child->ReadResource(file, version)) return 0;
			AddChild(child);
		}
		children.reverse();
		return 1;
	}
}
