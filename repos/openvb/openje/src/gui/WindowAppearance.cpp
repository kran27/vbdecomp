// Original Window appearance:4AF1B0/48,4AF1E0/11,4AF1F0/83,4AF250/15.
#include "gui/Window.hpp"

namespace GUI
{
	void Window::SetColor(DWORD value)
	{
		Fragment* fragment = fragments;
		int remaining = 9;
		do
		{
			fragment->SetColor(value);
			++fragment;
			--remaining;
		}
		while (remaining);
		color = value;
	}

	DWORD Window::GetColor()
	{
		return fragments[0].GetColor();
	}

	void Window::SetAlpha(BYTE value)
	{
		const DWORD alpha = static_cast<DWORD>(value) << 24;
		Fragment* fragment = fragments;
		int remaining = 9;
		do
		{
			fragment->SetColor((fragment->GetColor() & 0x00FFFFFF) | alpha);
			++fragment;
			--remaining;
		}
		while (remaining);
		color = (color & 0x00FFFFFF) | alpha;
	}

	DWORD Window::GetAlpha()
	{
		return fragments[4].GetColor() >> 24;
	}
	DWORD Window::GetMaterialModes() //4AF2D0, direct DWORD+64 getter.
	{
		return materialModes;
	}
}
