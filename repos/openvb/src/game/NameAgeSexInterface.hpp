#pragma once
#include "game/GameInterface.hpp"

namespace F3
{
	class CharacterGenerationInterface;
	class GamePlayer;
	class CounterControl;
	class NameAgeSexInterface : public GameInterface
	{
	public:
		NameAgeSexInterface(); //426720.
		virtual ~NameAgeSexInterface(); //425450.
		virtual int OnEvent(::GUI::Window*, ::GUI::Event*); //42FE40.
		virtual void BeforeUpdate(); //425500.
		//OnCursor425510 inherits the actual shared GameInterface policy.
		void Show(CharacterGenerationInterface*, GamePlayer*, const std::string&, LONG, LONG); //429760.
		void GetValues(std::string& text, LONG& years, LONG& gender); //426BF0, actual output-reference API.
		void Close(); //424A30.
		CharacterGenerationInterface* parent; //+04, borrowed.
		CounterControl* ageCounter; //+08, owned.
		GamePlayer* character; //+0C, borrowed.
		std::string name; //+10.
		LONG age, sex; //+2C/+30.
	private:
		void RefreshAge(); //424A20, actual ordinary counter-update boundary.
		void RefreshSex(); //426AC0.
	};
}
