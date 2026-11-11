#include "Window.hpp"
#include "Manager.hpp"

namespace GUI
{
	Window::Window(DWORD type)
		: hidden(0), frameVisible(1), centerVisible(1), value3B(0), value3C(0),
		color(0xFF808080), value60(0), materialModes(0), value68(0),
		owner(NULL), parent(NULL), valueCD4(0xFFFFFFFF), valueCF4(0xFFFFFFFF),
		viewport(NULL), type(type)
	{
		// The real bases, strings, list, nine Fragments and vector construct first.
		// Scalar padding and the members' unwritten fields retain their contents.
		SetRect(&rectangle, 0, 0, 96, 96);
		clientRectangle = rectangle;
		int i = 0;
		Fragment* fragment = fragments;
		for (; i < 9; ++i, ++fragment)
		{
			if (i & 1)
			{
				fragment->SetColor(0xFF804010);
				if (i == 1 || i == 7)
					fragment->Resize(64, 16);
				else
					fragment->Resize(16, 64);
			}
			else if (i != 4)
			{
				fragment->SetColor(0xFF408010);
				fragment->Resize(16, 16);
			}
		}
		fragments[4].Resize(64, 64);
		fragments[4].SetColor(0xFF808080);
		name = g_Manager.GetFragmentType(this->type);
	}

	Window::~Window()
	{
		std::list<Window*>::iterator entry = children.begin();
		while (entry != children.end())
		{
			Window* child = *entry;
			entry = children.erase(entry);
			// Original queries the secondary HeapTracker receiver before NULL.
			// NULL is outside this ordinary C++ member-call domain; retain the
			// observed order and separately check the pinned compiler's emission.
			if (child->IsHeapAllocated() && child)
				delete child;
		}
		if (parent)
			SetParent(NULL);
		if (owner)
			SetOwner(NULL);
		g_Manager.ForgetWindow(this);
		// Normal C++ destroys strings/vector/Fragments/list, then both bases.
	}
}
