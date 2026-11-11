#include "Window.hpp"

namespace GUI
{
	void Window::SetName(const std::string& value) { name = value; } //4B02C0.
	void Window::SetFrameVisible(BYTE value) { frameVisible = value; geometryDirty = 1; } //4AF2E0.
	BYTE Window::GetFrameVisible() { return frameVisible; } //4AF2F0.
	void Window::SetCenterVisible(BYTE value) { centerVisible = value; } //4AF810.
	BYTE Window::GetCenterVisible() { return centerVisible; } //4AF820.
	void Window::SetClientPosition(LONG x, LONG y) //4AF020, actual relative Offset callback.
	{
		Offset(static_cast<LONG>(DWORD(x)-DWORD(clientRectangle.left)),
			static_cast<LONG>(DWORD(y)-DWORD(clientRectangle.top)));
	}
	void Window::GetClientPosition(LONG& x, LONG& y) { x = clientRectangle.left; y = clientRectangle.top; }
	void Window::SetViewport(Gfx_Viewport* value) //4AF980, reverse live child traversal.
	{
		viewport = value;
		std::list<Window*>::iterator i = children.end();
		for (--i; i != children.end(); --i)
			(*i)->SetViewport(value);
	}
	Gfx_Viewport* Window::GetViewport() { return viewport; } //4AF4C0.
}
