#pragma once

#include "Window.hpp"

namespace GUI
{
	// Actual Window-derived owner: VC7 sizeof3448, tables66C9BC/66C9B4.
	class Button : public Window
	{
	public:
		explicit Button(DWORD type); // 4A6ED0.
		virtual ~Button(); // Ordinary4A6FE0; compiler supplies both deleting tables.
		virtual BYTE ReadResource(JE::Resources::ResourceFileHandle* file, int version); // 4A6B60.
		virtual BYTE ReadFile(FILE* file, int version); // 4A6A10; versioned state/keyboard/sounds, then Window.
		virtual BYTE WriteFile(FILE* file); // 4A6AC0; current state/keyboard/sounds, then Window.
		virtual void ApplySkin(const std::string& skin); // +CC,4A70D0.
		virtual BOOL OnEvent(Event* event); // +11C,4A6DA0.
		virtual DWORD GetState(); // Added slot+12C,4A6FA0.
		bool IsEnabled(); // 4A6D70; state3/7 disabled.
		BYTE SetKeyboardEnabled(BYTE value); //4A6D90; raw BYTE store/return.
		void SetEnabled(BYTE enabled); //4A6CE0, state family then live skin reapplication.
		void SetChecked(BYTE value); //4A6C10, raw checked byte and corresponding state family.
		void SetSoundDown(std::string value); //4435B0; callee owns incoming28B string.
		void SetSoundUp(std::string value); //443610.
		void SetSoundEnter(std::string value); //443670.

		DWORD state; // +D1C.
		BYTE checked, keyboardEnabled; // +D20/+D21; paddingD22..D23 unwritten.
		std::string soundDown, soundUp, soundEnter; // +D24/+D40/+D5C.
	};
}
