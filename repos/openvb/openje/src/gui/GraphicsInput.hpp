#pragma once

#include "MenuInput.hpp"
#include "Win32Input.hpp"

namespace JE
{
	namespace GUI
	{
		class GraphicsInputServices
		{
		public:
			virtual ~GraphicsInputServices() {}
			virtual BYTE Mouse(const MenuMouseState& mouse, DWORD modifiers) = 0;
			virtual BYTE Key(LONG key, BYTE down, DWORD modifiers);
			virtual BYTE Character(BYTE character, DWORD modifiers);
		};

		class MenuGraphicsInputServices : public GraphicsInputServices
		{
		public:
			MenuGraphicsInputServices(MenuInputState& state, MenuInputServices& services)
				: state(state), services(services) {}
			virtual BYTE Mouse(const MenuMouseState& mouse, DWORD modifiers)
			{
				return ProcessMenuMouse(state, mouse, modifiers, services);
			}
			MenuInputState& state;
			MenuInputServices& services;
		};

		BYTE DispatchGraphicsInput(const Input::InputEvent& event,
			GraphicsInputServices& services); //0x581230.
	}
}
