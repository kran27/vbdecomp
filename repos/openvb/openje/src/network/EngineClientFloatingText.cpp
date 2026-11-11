#include "EngineClient.hpp"
#include "graphics/FloatingTextInterface.hpp"
#include "startup/EngineStringSystem.hpp"
#include <stdexcept>

namespace JE
{
	ClientChild0xAC* Client::CreateChild0xAC() //58C140.
	{
		return new FloatingTextInterface("FloatingTextInterface");
	}

	void Client::AddFloatingSpeechText(DWORD entityID, DWORD stringReference, DWORD duration,
		DWORD red, DWORD green, DWORD blue) //58AAD0.
	{
		std::string text;
		F3::g_EngineStringSystem.GetString(stringReference, text); //Actual56C260, before owner reload.
		child0xAC->AddSpeech(entityID, text, duration, red, green, blue);
	}

	void Client::AddFloatingDamageText(DWORD entityID, const std::string& text,
		DWORD red, DWORD green, DWORD blue) //587A70.
	{
		child0xAC->AddDamage(entityID, text, red, green, blue);
	}
}
