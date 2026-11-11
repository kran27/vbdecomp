#include "Input.hpp"

namespace JE
{
	namespace Input
	{
		Input::Input()
		{
		}

		Input::~Input()
		{
		}

		void Input::PushEvent(const InputEvent& event)
		{
			events.push_back(event);
		}

		bool Input::PopEvent(InputEvent& output)
		{
			if (events.empty()) return false;
			output = events.front();
			events.pop_front();
			return true;
		}

		void Input::Cleanup()
		{
		}
	}
}
