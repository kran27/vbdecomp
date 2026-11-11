#include "Base.hpp"

namespace GUI
{
	Base::Base()
	{
		geometryDirty = 1;
		::SetRect(&rectangle, 0, 0, 64, 64);
	}

	void Base::SetPosition(LONG x, LONG y)
	{
		Offset(x - rectangle.left, y - rectangle.top);
	}

	void Base::GetPosition(LONG& x, LONG& y)
	{
		x = rectangle.left;
		y = rectangle.top;
	}

	void Base::Offset(LONG x, LONG y)
	{
		geometryDirty = 1;
		::OffsetRect(&rectangle, x, y);
	}

	void Base::Resize(LONG width, LONG height)
	{
		if (width >= 2 && height >= 2)
		{
			LONG right = rectangle.left + width;
			LONG bottom = rectangle.top + height;
			rectangle.right = right;
			geometryDirty = 1;
			rectangle.bottom = bottom;
		}
	}

	void Base::GetSize(LONG& width, LONG& height)
	{
		width = rectangle.right - rectangle.left;
		height = rectangle.bottom - rectangle.top;
	}

	bool Base::Contains(LONG x, LONG y)
	{
		POINT point = { x, y };
		return ::PtInRect(&rectangle, point) != 0;
	}

	BYTE Base::ReadFile(FILE* file, int)
	{
		if (std::fread(&rectangle, 16, 1, file) != 1) return 0;
		geometryDirty = 1;
		return 1;
	}

	BYTE Base::WriteFile(FILE* file)
	{
		return std::fwrite(&rectangle, 16, 1, file) == 1;
	}

	void Base::Refresh()
	{
		if (geometryDirty) UpdateGeometry();
	}

	void Base::Draw() {}

	void Base::UpdateGeometry()
	{
		geometryDirty = 0;
	}
}
