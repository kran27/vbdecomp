#pragma once

#include <windows.h>
#include <cstddef>
#include <deque>

namespace JE
{
	namespace Input
	{
		struct InputEvent
		{
			DWORD category; //0 key,1 character,2 mouse.
			DWORD modifiers; //Control1,Shift2,Alt4.
			union { DWORD subtype; BYTE character; };
			LONG x, y; //Key events use x for the9-bit scan code.
			LONG deltaX, deltaY, wheel;
			DWORD button; //Left0,middle1,right2.
			BYTE left, middle, right;
			// Final padding and inactive category fields remain unwritten.
		};
		typedef char InputEventSizeRequired[sizeof(InputEvent) == 40 ? 1 : -1];
		typedef char InputEventOffsetsRequired[(offsetof(InputEvent, x) == 12
			&& offsetof(InputEvent, left) == 36) ? 1 : -1];

		class Input
		{
		public:
			Input(); //5C1EC0.
			virtual ~Input(); //5C1EE0.
			virtual void PushEvent(const InputEvent& event); //5C2190.
			virtual bool PopEvent(InputEvent& output); //5C1EF0.
			virtual const char* KeyName(DWORD scanCode) const = 0;
			virtual DWORD FindKey(const char* name) const = 0;
			virtual void Initialize() = 0;
			virtual void Cleanup(); //Shared empty404B70.

			std::deque<InputEvent> events;
		private:
			Input(const Input&);
			Input& operator=(const Input&);
		};
		#if defined(_MSC_VER) && _MSC_VER == 1300
		typedef char InputNativeSizeRequired[sizeof(Input) == 24 ? 1 : -1];
		typedef char InputNativeQueueRequired[offsetof(Input, events) == 4 ? 1 : -1];
		#endif
	}
}
