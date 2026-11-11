#pragma once

#include <windows.h>

namespace GUI { class Interface; class Picture; }

namespace JE
{
	class FadeManager
	{
	public:
		FadeManager(); //5B4A60, actual Client+A8 owner.
		virtual ~FadeManager(); //5B4C60; deliberately does not delete its GUI owners.
		virtual void Initialize(); //Shared404B70.
		virtual void InitializeUI(); //5B4CC0.
		virtual void OnCleanup(); //Shared404B70.
		void FadeIn(DWORD unusedDuration, LONG red, DWORD green, DWORD blue); //588CE0 ->5B4B30.
		void FadeOut(DWORD unusedDuration, LONG red, DWORD green, DWORD blue); //588CF0 ->5B4BD0.
		virtual void Update(); //5B4C70 ->5B4A80/5B4AE0, original slot+10.

		::GUI::Interface* interfaceOwner; //Original+04.
		::GUI::Picture* picture; //Original+08, owned by interface's GUI policy.
		DWORD mode, started, duration; //+0C/+10/+14.

	private:
		void UpdateFadeIn(); //5B4A80, tail-dispatched by Update.
		void UpdateFadeOut(); //5B4AE0, tail-dispatched by Update.
	};
}
