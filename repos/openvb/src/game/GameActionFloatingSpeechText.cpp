#include "GameActionFloatingSpeechText.hpp"

namespace F3
{
	GameActionFloatingSpeechText::GameActionFloatingSpeechText() {}
	int GameActionFloatingSpeechText::Start(DWORD time)
	{
		JE::ActionFloatingSpeechText::Start(time);
		return 0;
	}
}
