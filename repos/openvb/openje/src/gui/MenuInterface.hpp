#pragma once

#include "MenuInput.hpp"

namespace JE
{
	namespace GUI
	{
		// Normal native API handle. This is not a reconstructed CSound layout
		// or a successful sound backend; adapters own their actual resource.
		class MenuSound
		{
		public:
			virtual ~MenuSound() {}
		protected:
			MenuSound() {}
		};
		class MenuInterfaceServices
		{
		public:
			virtual ~MenuInterfaceServices() {}
			virtual int LocalNodeEvent(MenuNode& node, MenuEvent& event) = 0;
			virtual DWORD ButtonState(MenuNode& node);
			virtual std::string NodeHint(MenuNode& node);
			virtual bool ResolveHint(DWORD reference, std::string& output);
			virtual void SetTooltip(const std::string& text) = 0;

			// Whole CSound construction and sound APIs are explicit required
			// children. The deferred backend is never represented by success.
			virtual MenuSound* CreateSound();
			virtual void LoadSound(MenuSound& sound, const char* name, DWORD flags);
			virtual void PlaySound(MenuSound& sound, DWORD mode, DWORD arg1, DWORD arg2);
		};

		std::string GetMenuNodeHint(MenuNode& node, MenuInterfaceServices& services); //0x4B0040.
		int HandleBaseInterfaceEvent(MenuNode* node, MenuEvent& event,
			MenuInterfaceServices& services); //0x4958D0.

		class MenuInterface : public MenuEventOwner
		{
		public:
			MenuInterface(MenuLayout& layout, MenuInterfaceServices& services)
				: layout(layout), services(services) {}
			int HandleEvent(MenuNode& node, MenuEvent& event); //0x5803E0.
			MenuLayout& layout;
			MenuInterfaceServices& services;
		};
	}
}
