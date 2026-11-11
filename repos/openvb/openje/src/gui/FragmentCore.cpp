#include "Fragment.hpp"

namespace GUI
{
	void Fragment::Resize(LONG width, LONG height)
	{
		Base::Resize(width, height);
		uvDirty = 1;
	}

	void Fragment::SetTileMode(DWORD mode)
	{
		if (mode <= 7)
		{
			tileMode = mode;
			uvDirty = 1;
		}
	}

	DWORD Fragment::GetColor() { return color; }

	void Fragment::SetFrameRate(DWORD rate) { frameRate = rate; }

	DWORD Fragment::GetTileMode() { return tileMode; }
	DWORD Fragment::GetFrameRate() { return frameRate; }
	DWORD Fragment::GetFrameCount() { return frameCount; }
	void Fragment::SetFrameSize(DWORD width, DWORD height)
	{
		frameWidth = width;
		frameHeight = height;
	}

	void Fragment::GetFrameSize(DWORD& width, DWORD& height)
	{
		width = frameWidth;
		height = frameHeight;
	}

	void Fragment::SetFrameCount(DWORD count) { frameCount = count; }

	void Fragment::SetSourceRect(float x, float y, float width, float height)
	{
		sourceX = x;
		sourceY = y;
		sourceWidth = width;
		sourceHeight = height;
		geometryDirty = 1;
	}

	void Fragment::GetSourceRect(float& x, float& y, float& width, float& height)
	{
		x = sourceX;
		y = sourceY;
		width = sourceWidth;
		height = sourceHeight;
	}

	void Fragment::Refresh() { Base::Refresh(); }

	float Fragment::SetDepth(float depth, float)
	{
		for (int i = 0; i < 4; ++i) vertices[i].z = depth;
		return depth;
	}

	float Fragment::GetDepth() { return vertices[0].z; }
}
