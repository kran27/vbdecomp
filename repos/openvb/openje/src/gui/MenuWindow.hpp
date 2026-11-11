#pragma once

#include "MenuLayout.hpp"

namespace JE
{
	namespace GUI
	{
		class MenuWindowServices
		{
		public:
			virtual ~MenuWindowServices() {}
			virtual void Size(MenuFragment& fragment, LONG& width, LONG& height);
			virtual void Position(MenuFragment& fragment, LONG x, LONG y);
			virtual void Resize(MenuFragment& fragment, LONG width, LONG height);
			virtual void UpdateFragment(MenuFragment& fragment) = 0;
			virtual void DrawFragment(MenuFragment& fragment, Gfx::Viewport* viewport, BYTE flags) = 0;
			virtual void LayoutNode(MenuNode& node);
			virtual void UpdateChild(MenuNode& node);
			virtual void DrawChild(MenuNode& node);
		};

		class NativeMenuWindowServices : public MenuWindowServices
		{
		public:
			explicit NativeMenuWindowServices(FragmentServices& fragments) : fragments(fragments) {}
			void UpdateFragment(MenuFragment& fragment);
			void DrawFragment(MenuFragment& fragment, Gfx::Viewport* viewport, BYTE flags);
			FragmentServices& fragments;
		};

		// Selected actual Window/Button/Picture table policies, not full GUI lifetime.
		void LayoutMenuWindow(MenuNode& node, MenuWindowServices& services); //0x4AF5B0.
		void UpdateMenuWindow(MenuNode& node, MenuWindowServices& services); //0x4AFA90.
		void DrawMenuWindow(MenuNode& node, MenuWindowServices& services); //0x4AFB30.
	}
}
