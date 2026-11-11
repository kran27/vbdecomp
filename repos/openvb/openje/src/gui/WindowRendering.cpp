// Recovered Window policy; complete matching status is recorded separately.
#include "gui/Window.hpp"

namespace
{
	LONG Add(LONG a, LONG b)
	{
		return static_cast<LONG>(static_cast<DWORD>(a) + static_cast<DWORD>(b));
	}
	LONG Subtract(LONG a, LONG b)
	{
		return static_cast<LONG>(static_cast<DWORD>(a) - static_cast<DWORD>(b));
	}
}

namespace GUI
{
	// 0x4AF830
	void Window::Offset(LONG x, LONG y)
	{
		Base::Offset(x, y);
		::OffsetRect(&clientRectangle, x, y);
		for (std::list<Window*>::iterator entry = children.begin(); entry != children.end(); ++entry)
			(*entry)->Offset(x, y);
	}

	// 0x4AF5A0
	float Window::GetDepth()
	{
		return fragments[0].GetDepth();
	}

	// 0x4AFA90
	void Window::Refresh()
	{
		Base::Refresh();
		if (!hidden)
		{
			if (frameVisible)
				for (int i = 0; i < 4; ++i)
					fragments[i].Refresh();
			if (centerVisible)
				fragments[4].Refresh();
			if (frameVisible)
				for (int j = 5; j < 9; ++j)
					fragments[j].Refresh();
			std::list<Window*>::iterator entry = children.end();
			--entry;
			while (entry != children.end())
			{
				(*entry)->Refresh();
				--entry;
			}
		}
	}

	// 0x4AFB30
	void Window::Draw()
	{
		if (!hidden)
		{
			DWORD flags = (value60 ? 1 : 0) | (value68 ? 2 : 0);
			if (frameVisible)
				for (int i = 0; i < 4; ++i)
					fragments[i].Draw(viewport, flags);
			if (centerVisible)
				fragments[4].Draw(viewport, flags);
			if (frameVisible)
				for (int j = 5; j < 9; ++j)
					fragments[j].Draw(viewport, flags);
			std::list<Window*>::iterator entry = children.end();
			--entry;
			while (entry != children.end())
			{
				(*entry)->Draw();
				--entry;
			}
		}
	}

	// 0x4AF5B0
	void Window::UpdateGeometry()
	{
		Base::UpdateGeometry();
		LONG widths[9], heights[9];
		for (int i = 0; i < 9; ++i)
		{
			if (frameVisible || i == 4)
				fragments[i].GetSize(widths[i], heights[i]);
			else
				widths[i] = heights[i] = 0;
		}
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
		const LONG x = rectangle.left;
		if (Subtract(rectangle.right, x) < Add(right, left))
		{
			const LONG newRight = Add(Add(Add(right, x), left), 1);
			const LONG clientOffset = Subtract(clientRectangle.left, x);
			rectangle.right = newRight;
			clientRectangle.right = Add(newRight, clientOffset);
		}
		const LONG y = rectangle.top;
		if (Subtract(rectangle.bottom, y) < Add(top, bottom))
		{
			const LONG newBottom = Add(Add(Add(bottom, y), top), 1);
			const LONG clientOffset = Subtract(clientRectangle.top, y);
			rectangle.bottom = newBottom;
			clientRectangle.bottom = Add(newBottom, clientOffset);
		}
		left = Add(left, x);
		top = Add(top, y);
		right = Subtract(rectangle.right, right);
		bottom = Subtract(rectangle.bottom, bottom);
		const LONG innerWidth = Subtract(right, left);
		const LONG innerHeight = Subtract(bottom, top);
		fragments[0].SetPosition(Subtract(left, widths[0]), Subtract(top, heights[0]));
		fragments[1].SetPosition(left, Subtract(top, heights[1]));
		fragments[1].Resize(innerWidth, heights[1]);
		fragments[2].SetPosition(right, Subtract(top, heights[2]));
		fragments[3].SetPosition(Subtract(left, widths[3]), top);
		fragments[3].Resize(widths[3], innerHeight);
		fragments[4].SetPosition(left, top);
		fragments[4].Resize(innerWidth, innerHeight);
		fragments[5].SetPosition(right, top);
		fragments[5].Resize(widths[5], innerHeight);
		fragments[6].SetPosition(Subtract(left, widths[6]), bottom);
		fragments[7].SetPosition(left, bottom);
		fragments[7].Resize(innerWidth, heights[7]);
		fragments[8].SetPosition(right, bottom);
	}
}
