#pragma once

#include <winsock2.h>
#include <windows.h>
#include <string>

namespace JE { class Client; }
namespace F3
{
	class FeedbackInterface;
	class ActionbarInterface;
	//Typed projection of the actual standalone Feedback while the full
	//GameInterfaces parent remains unbound. It never creates a missing HUD owner.
	FeedbackInterface* GetClientFeedback(JE::Client* client);
	ActionbarInterface* GetFeedbackActionbar();
	void DisplayClientFeedback(std::string text,DWORD red,DWORD green,DWORD blue,std::string speaker);
}
