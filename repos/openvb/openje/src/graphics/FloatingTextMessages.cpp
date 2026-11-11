#include "graphics/FloatingTextInterface.hpp"

namespace JE
{
	void FloatingTextInterface::AddDamage(DWORD id, const std::string& text, DWORD red, DWORD green, DWORD blue)
	{ Add(id, text, red, green, blue, damage.profile, damage.font, damage.pointSize, damage.fontFlags,
		damage.duration, damage.speed, damage.fadeOut, damage.fadeIn, damage.width, damage.height,
		damage.wrap, damage.alignment, 2); }
	void FloatingTextInterface::AddSpeech(DWORD id, const std::string& text, DWORD duration,
		DWORD red, DWORD green, DWORD blue)
	{
		for (int i = 0; i < 32; ++i) if (labels[i].entityID == id) labels[i].duration = 0;
		if (static_cast<LONG>(duration) <= 0) duration = GetReadingTime(text);
		Add(id, text, red, green, blue, speech.profile, speech.font, speech.pointSize, speech.fontFlags,
			duration, speech.speed, speech.fadeOut, speech.fadeIn, speech.width, speech.height,
			speech.wrap, speech.alignment, 1);
	}
}
