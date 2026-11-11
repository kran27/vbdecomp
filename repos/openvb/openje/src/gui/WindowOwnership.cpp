#include "Window.hpp"
#include "Interface.hpp"
#include "Manager.hpp"
#include <algorithm>

namespace GUI
{
	void Window::BringChildToFront(Window* window) //4B0D50, actual erase/reinsert at head.
	{
		for (std::list<Window*>::iterator entry = children.begin();
			entry != children.end(); ++entry)
		{
			if (window == *entry)
			{
				children.erase(entry);
				children.push_front(window);
				g_Manager.MarkDepthDirty();
				return;
			}
		}
	}
	void Window::Resize(LONG width, LONG height)
	{
		if (frameVisible)
		{
			LONG widths[9], heights[9];
			for (int i = 0; i < 9; ++i)
				fragments[i].GetSize(widths[i], heights[i]);
			LONG left = 0, top = 0, right = 0, bottom = 0;
			int j = 0;
			LONG* rowWidths = widths;
			for (; j < 3; ++j, rowWidths += 3)
			{
				if (rowWidths[0] > left)
					left = rowWidths[0];
				if (heights[j] > top)
					top = heights[j];
				if (rowWidths[2] > right)
					right = rowWidths[2];
				if (heights[j + 6] > bottom)
					bottom = heights[j + 6];
			}
			const LONG minimumWidth = static_cast<LONG>(static_cast<DWORD>(left) + static_cast<DWORD>(right));
			if (width < minimumWidth)
				width = minimumWidth;
			const LONG minimumHeight = static_cast<LONG>(static_cast<DWORD>(top) + static_cast<DWORD>(bottom));
			if (height < minimumHeight)
				height = minimumHeight;
		}
		else
		{
			if (width < 2)
				width = 2;
			if (height < 2)
				height = 2;
		}
		clientRectangle.right = static_cast<LONG>(static_cast<DWORD>(clientRectangle.left) + static_cast<DWORD>(width));
		clientRectangle.bottom = static_cast<LONG>(static_cast<DWORD>(clientRectangle.top) + static_cast<DWORD>(height));
		Base::Resize(width, height);
	}

	void Window::SetOwner(Interface* value)
	{
		if (owner)
			owner->RemoveWindow(this);
		if (parent)
			SetParent(NULL);
		owner = value;
		if (value)
			value->AddWindow(this);
		clientRectangle = rectangle;
		g_Manager.MarkDepthDirty();
	}

	Interface* Window::GetOwner()
	{
		return owner;
	}

	void Window::SetParent(Window* value)
	{
		if (ContainsDescendant(value))
		{
			if (parent)
				value->SetParent(parent);
			else if (owner)
				value->SetOwner(owner);
			else
				value->SetParent(NULL);
		}
		if (parent)
			parent->RemoveChild(this);
		if (owner)
			owner->RemoveWindow(this);
		parent = value;
		if (value)
		{
			value->AddChild(this);
			LONG x, y;
			parent->GetAbsolutePosition(x, y);
			const DWORD left = static_cast<DWORD>(rectangle.left) - static_cast<DWORD>(x);
			const DWORD top = static_cast<DWORD>(rectangle.top) - static_cast<DWORD>(y);
			clientRectangle.right = static_cast<LONG>(static_cast<DWORD>(clientRectangle.right)
				+ left - static_cast<DWORD>(clientRectangle.left));
			clientRectangle.bottom = static_cast<LONG>(static_cast<DWORD>(clientRectangle.bottom)
				+ top - static_cast<DWORD>(clientRectangle.top));
			clientRectangle.left = static_cast<LONG>(left);
			clientRectangle.top = static_cast<LONG>(top);
		}
		g_Manager.MarkDepthDirty();
	}

	Window* Window::GetParent()
	{
		return parent;
	}

	void Window::AddChild(Window* value)
	{
		if (value->GetParent() != this)
			value->SetParent(this);
		for (std::list<Window*>::iterator entry = children.begin();
			entry != children.end(); ++entry)
		{
			if (value == *entry)
			{
				children.erase(entry);
				break;
			}
		}
		children.push_front(value);
	}

	void Window::RemoveChild(Window* value)
	{
		for (std::list<Window*>::iterator entry = children.begin();
			entry != children.end(); ++entry)
		{
			if (value == *entry)
			{
				children.erase(entry);
				value->SetParent(NULL);
				return;
			}
		}
	}

	bool Window::ContainsDescendant(Window* value)
	{
		for (std::list<Window*>::iterator entry = children.begin();
			entry != children.end(); ++entry)
		{
			Window* child = *entry;
			if (value == child || child->ContainsDescendant(value))
				return true;
		}
		return false;
	}
}
