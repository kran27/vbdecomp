#include "Manager.hpp"
#include <cstring>

namespace GUI
{
	BYTE Manager::WriteClipboardText(const std::string& text) //Manager4B2CB0.
	{
		if (!OpenClipboard(NULL)) return 0;
		if (!EmptyClipboard()) { CloseClipboard(); return 0; }
		HGLOBAL data = GlobalAlloc(0x2002, text.size() + 1);
		char* output = static_cast<char*>(GlobalLock(data));
		if (output) { std::strcpy(output, text.c_str()); GlobalUnlock(data); }
		BYTE result = SetClipboardData(CF_TEXT, data) != NULL;
		CloseClipboard(); return result; //No original failure-free or invented ownership repair.
	}

	void Manager::SetTooltip(const std::string& text) //4B4160.
	{
		tooltip = text;
	}

	void Manager::GetResolution(DWORD& width, DWORD& height)
	{
		width = resolutionWidth;
		height = resolutionHeight;
	}

	void Manager::GetScale(float& x, float& y)
	{
		x = scaleX;
		y = scaleY;
	}

	void Manager::GetMouseState(MouseState& state)
	{
		state = mouse;
	}

	void Manager::ResetMouseState()
	{
		mouse.x = 0;
		mouse.y = 0;
		mouse.buttons = 0;
	}
}
