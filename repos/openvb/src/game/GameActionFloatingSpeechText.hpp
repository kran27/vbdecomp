#pragma once
#include "entity/ActionFloatingSpeechText.hpp"

namespace F3
{
	class GameActionFloatingSpeechText : public JE::ActionFloatingSpeechText
	{
	public:
		GameActionFloatingSpeechText(); //405240; base55FB60 plus actual derived table.
		virtual int Start(DWORD time); //405270, qualified base no-op then0.
	};
}
