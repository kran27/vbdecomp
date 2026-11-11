#include "gui/Interfaces.hpp"

namespace JE
{
	Interfaces::Interfaces() {}
	Interfaces::~Interfaces() {}
	void Interfaces::Initialize() {}
	void Interfaces::Activate() {}
	void Interfaces::FinishInitialization() {}
	void Interfaces::SetLoadingVisible(DWORD) {}
	void Interfaces::ShowGameplay() {}
	void Interfaces::HideGameplay() {}
	void Interfaces::RefreshSelection() {}
	void Interfaces::GetScreenRectangle(LONG& x, LONG& y, LONG& width, LONG& height)
	{
		x = 0;
		y = 0;
		width = 1024;
		height = 768;
	}
	void Interfaces::OnUnused() {}
}
