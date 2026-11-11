// Isolated actual Window accessor candidates; existing slots+B8 and+128.
#include "gui/Window.hpp"

namespace GUI
{
	void Window::SetValueCD4(DWORD value) { valueCD4=value; } //4AF580.
	void Window::SetValueCF4(DWORD value) { valueCF4=value; } //4AF590.
	void Window::SetStringCF8(const std::string& value) { stringCF8=value; } //4B0680.
	std::string Window::GetStringCF8() { return stringCF8; } //4B0280, real string copy.
	// 0x4AF470/7; return the member container, not its sentinel pointer.
	std::list<Window*>* Window::GetChildren()
	{
		return &children;
	}

	// Shared0x420990/6.
	DWORD Window::GetDepthSlots()
	{
		return 1;
	}
}
