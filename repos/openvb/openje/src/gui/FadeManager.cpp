#include "gui/FadeManager.hpp"
#include "gui/Interface.hpp"
#include "gui/Picture.hpp"
#include "graphics/SystemInterface.hpp"
#include <mmsystem.h>

namespace JE
{
	FadeManager::FadeManager() : interfaceOwner(NULL), picture(NULL), mode(0), started(0), duration(0) {}
	FadeManager::~FadeManager() {}
	void FadeManager::Initialize() {}
	void FadeManager::OnCleanup() {}

	void FadeManager::InitializeUI()
	{
		if (!interfaceOwner) interfaceOwner = new ::GUI::Interface("FadeInterface");
		if (!picture)
		{
			Gfx_Viewport* viewport = g_GfxSystem->PostViewport(); //Actual engine slot+98.
			picture = new ::GUI::Picture(2);
			picture->Resize(1024, 768);
			picture->ApplySkin("DefaultInterface.ini");
			picture->SetColor(0xFF000000);
			picture->SetFrameVisible(0);
			picture->SetHidden(1);
			picture->SetValue3B(1);
			picture->SetViewport(viewport);
			picture->SetAbsolutePosition(0, 0);
			interfaceOwner->AddWindow(picture);
		}
	}

	void FadeManager::FadeIn(DWORD, LONG red, DWORD green, DWORD blue)
	{
		if (!interfaceOwner || !picture) return;
		picture->SetColor(D3DCOLOR_XRGB(red, green, blue));
		picture->SetAlpha(255);
		mode = 1;
		started = timeGetTime();
		//The physical original reads argument1 here, not argument0. Preserve it:
		//the menu's(1000,0,0,0) request completes immediately.
		duration = DWORD(red);
		if (static_cast<LONG>(duration) <= 0)
		{
			picture->SetAlpha(0);
			picture->SetHidden(1);
			mode = 0;
		}
		else picture->SetHidden(0);
	}

	void FadeManager::FadeOut(DWORD, LONG red, DWORD green, DWORD blue)
	{
		if (!interfaceOwner || !picture) return;
		picture->SetColor(D3DCOLOR_XRGB(red, green, blue));
		picture->SetAlpha(0);
		mode = 2;
		started = timeGetTime();
		duration = DWORD(red);
		if (static_cast<LONG>(duration) <= 0)
		{
			picture->SetAlpha(255);
			mode = 0; //Unlike FadeIn, this branch does not write hidden.
		}
		else picture->SetHidden(0);
	}

	void FadeManager::UpdateFadeIn()
	{
		const DWORD elapsed = timeGetTime() - started;
		if (!elapsed) picture->SetAlpha(255);
		else if (elapsed > duration)
		{
			picture->SetAlpha(0);
			picture->SetHidden(1);
			mode = 0;
		}
		else picture->SetAlpha(BYTE(255u * (duration - elapsed) / duration));
	}

	void FadeManager::UpdateFadeOut()
	{
		const DWORD elapsed = timeGetTime() - started;
		if (!elapsed) picture->SetAlpha(0);
		else if (elapsed > duration)
		{
			picture->SetAlpha(255);
			mode = 0;
		}
		else picture->SetAlpha(BYTE(255u * elapsed / duration));
	}

	void FadeManager::Update()
	{
		if (!interfaceOwner || !picture) return;
		switch (mode)
		{
		case 1: UpdateFadeIn(); break;
		case 2: UpdateFadeOut(); break;
		}
	}
}
