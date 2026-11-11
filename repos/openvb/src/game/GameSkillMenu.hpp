#pragma once

#include "gui/EngineInterface.hpp"
#include "game/GameInterface.hpp"
#include "gui/Label.hpp"
#include <d3dx8.h>
#include <list>

namespace JE
{
	class Client;

	class CSMenuLabel : public ::GUI::Label
	{
	public:
		CSMenuLabel(); //568CA0.
		virtual ~CSMenuLabel(); //568430; actual derived interface-table destruction phase.
		virtual BOOL OnEvent(::GUI::Event* event); //568450.
		virtual void SetDisabled(BYTE value); //5684B0, original terminal slot+1A8.

		DWORD selector, valueE68, valueE6C; //Original+E64/+E68/+E6C.
		DWORD actorID, targetID; //+E70/+E74.
		BYTE disabled; //+E78.
	};

	class CSMenuInterface : virtual public EngineInterface
	{
	public:
		explicit CSMenuInterface(const std::string& name); //568E50.
		virtual ~CSMenuInterface(); //568A70, then actual EngineInterface lifetime.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //5687B0.
		virtual void BeforeUpdate(); //568550.
		virtual void Populate(DWORD actorID, DWORD targetID); //Original primary+00.
		void Show(DWORD actorID, DWORD targetID, LONG x, LONG y,
			DWORD delay, DWORD fadeDuration); //568970.
		void Close(); //568510, hides only the embedded root.
		void Clear(); //568900, owns labels and leaves the other state intact.
		CSMenuLabel* AddLabel(const std::string& text, DWORD actorID, DWORD targetID,
			DWORD selector, DWORD valueE68, DWORD valueE6C, BYTE disabled); //569080.
		CSMenuLabel* FindSkillLabel(DWORD selector) const; //Borrowed observation API.

		::GUI::Window rootWindow; //Actual embedded Window, not a separate popup owner.
		LONG maximumTextWidth; //Original+D24.
		std::list<CSMenuLabel*> labels; //+D28, owns labels; Window borrows their linkage.
		D3DXVECTOR3 anchor; //+D34, constructor zero.
		DWORD fadeStart, fadeDuration, actorID, targetID; //+D40..D4C.
	private:
		void Fade(DWORD time); //568710.
		CSMenuInterface(const CSMenuInterface&);
		CSMenuInterface& operator=(const CSMenuInterface&);
	};
}

namespace F3
{
	class GameCSSkillMenuInterface : public GameInterface, public JE::CSMenuInterface
	{
	public:
		explicit GameCSSkillMenuInterface(const std::string& name); //435C90.
		virtual ~GameCSSkillMenuInterface(); //435D50, actual derived destruction phase.
		virtual void Populate(DWORD actorID, DWORD targetID); //435FA0.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //435DE0.
	};

	class GameSkillMenu : public GameInterface, public JE::CSMenuInterface
	{
	public:
		explicit GameSkillMenu(const std::string& name); //435840, actual GameCSMenuInterface.
		virtual ~GameSkillMenu(); //4353C0, actual derived destruction phase.
		virtual void Populate(DWORD actorID, DWORD targetID); //4359F0.
		virtual int OnEvent(::GUI::Window* window, ::GUI::Event* event); //435480, thunk435810.
		void Open(DWORD actorID, DWORD targetID, LONG x, LONG y); //4357C0.

		GameCSSkillMenuInterface skills; //Original independent embedded owner at+D54.
	};

}
