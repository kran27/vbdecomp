#include "MenuInterface.hpp"

#include <stdexcept>

namespace JE
{
	namespace GUI
	{
		namespace
		{
			void PlayNamedSound(MenuNode& node, unsigned index, MenuInterfaceServices& services)
			{
				// The original owns a local string copy before creating the sound;
				// callbacks may change the node's live sound name afterward.
				const std::string name = node.buttonSounds[index];
				if (!name.empty())
				{
					MenuSound* sound = services.CreateSound();
					if (sound)
					{
						services.LoadSound(*sound, name.c_str(), 0);
						services.PlaySound(*sound, 2, 0, 0);
					}
				}
			}
		}

		DWORD MenuInterfaceServices::ButtonState(MenuNode& node)
		{
			return node.button0xD1C;
		}

		std::string GetMenuNodeHint(MenuNode& node, MenuInterfaceServices& services)
		{
			if (!node.field0xCB8.empty())
				return node.field0xCB8;
			std::string resolved;
			if (node.field0xCD4 != 0xFFFFFFFFu && services.ResolveHint(node.field0xCD4, resolved))
				return resolved;
			return node.name;
		}

		std::string MenuInterfaceServices::NodeHint(MenuNode& node)
		{
			return GetMenuNodeHint(node, *this);
		}

		int HandleBaseInterfaceEvent(MenuNode* node, MenuEvent& event, MenuInterfaceServices& services)
		{
			if (event.type == 1)
			{
				if (node)
					services.SetTooltip(services.NodeHint(*node));
			}
			else if (event.type == 3)
				services.SetTooltip(std::string());
			return 0;
		}

		int MenuInterface::HandleEvent(MenuNode& node, MenuEvent& event)
		{
			const int type = event.type;
			if (type == 4 || type == 7)
			{
				if (node.type == 1)
					PlayNamedSound(node, 0, services);
			}
			else if (type == 5 || type == 3)
			{
				if (node.type == 1 && services.ButtonState(node) == 2)
					PlayNamedSound(node, 1, services);
			}
			else if (type == 1 && node.type == 1)
			{
				HandleBaseInterfaceEvent(&node, event, services);
				services.LocalNodeEvent(node, event);
				if (services.ButtonState(node) == 2)
					PlayNamedSound(node, 0, services);
				return 1;
			}
			return HandleBaseInterfaceEvent(&node, event, services);
		}

		bool MenuInterfaceServices::ResolveHint(DWORD, std::string&)
		{
			throw std::logic_error("Unrecovered GUI hint resolver0x4B4180");
		}

		MenuSound* MenuInterfaceServices::CreateSound()
		{
			throw std::logic_error("Unrecovered CSound construction0x4C8100");
		}

		void MenuInterfaceServices::LoadSound(MenuSound&, const char*, DWORD)
		{
			throw std::logic_error("Unrecovered CSound load API called at0x5804BB");
		}

		void MenuInterfaceServices::PlaySound(MenuSound&, DWORD, DWORD, DWORD)
		{
			throw std::logic_error("Unrecovered CSound play0x4C7EB0");
		}
	}
}
