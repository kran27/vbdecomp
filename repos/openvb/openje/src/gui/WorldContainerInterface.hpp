#pragma once

#include "gui/Interface.hpp"
#include "gui/WorldContainerServices.hpp"
#include "audio/Sound.hpp"
#include <d3dx8.h>
#include <vector>

namespace GUI { class Window; class Picture; class Button; class ScrollBar; }
namespace F3 { class GameItem; class GameInventory; }

namespace JE
{
	struct WorldContainerRow
	{
		void Bind(F3::GameItem* value); //457460; NULL leaves the old texture intact.
		::GUI::Picture* picture;
		F3::GameItem* item; //Borrowed; inventory owns the item.
	};

	class WorldContainerInterface : public ::GUI::Interface
	{
	public:
		explicit WorldContainerInterface(const std::string& name); //457DB0.
		virtual ~WorldContainerInterface(); //457D20.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //4575D0; sibling source.
		virtual void BeforeUpdate(); //4577D0.
		void Populate(DWORD target, DWORD actor); //4580C0.
		void Refresh(); //458710; repeat Populate with the current IDs.
		void Clear(); //457A10; leaves row bindings/IDs/anchor unchanged.
		F3::GameInventory* GetInventory(); //4574B0; target lookup and actual RTTI.
		F3::GameItem* FindRowItem(::GUI::Window* window); //4574F0; pointer identity.
		void RefreshRows(); //457520; requires the actual nonnull ScrollBar and row Pictures.
		void SetAbsolutePosition(LONG x, LONG y); //4574A0; root delegate.

		::GUI::Window* rootWindow; //Original+34, borrowed from Interface roots.
		LONG baseWidth, baseHeight; //+38/+3C.
		CSound clickSound; //+40..137; actual embedded owner, independent of GUI RTTI ancestry.
		DWORD targetID, actorID; //+138/+13C.
		std::vector<F3::GameItem*> items; //+140; vector owns storage, borrows all elements.
		WorldContainerRow rows[10]; //+150..19F.
		::GUI::ScrollBar* scrollbar; //+1A0; fresh normal branch remains NULL.
		::GUI::Button* upButton;
		::GUI::Button* downButton; //+1A4/+1A8.
		D3DXVECTOR3 anchor; //+1AC..1B7; world position captured by Populate.
	private:
		WorldContainerInterface(const WorldContainerInterface&);
		WorldContainerInterface& operator=(const WorldContainerInterface&);
	};
}
