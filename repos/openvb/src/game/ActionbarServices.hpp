#pragma once

#include <winsock2.h>
#include <windows.h>
#include <string>

namespace JE { struct ClientInputState; }
namespace F3
{
	struct GameClient;
	namespace ActionbarServices
	{
		GameClient* GetClient();
		JE::ClientInputState& InputState();
		HWND Window();
		void SetCompanionHidden(DWORD ownerOffset, BYTE hidden);
		void RefreshMinimap();
		void Feedback(const std::string& text, DWORD red, DWORD green, DWORD blue);
		void QueueCommand(const char* command); //580D10, actual Client+7C.
	}
}
