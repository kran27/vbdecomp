#pragma once
#include <winsock2.h>
#include "game/GameInterface.hpp"
#include <string>

class AreaMapWindow;
namespace JE { class Map; }

namespace F3
{
	class MiniMapInterface : public GameInterface
	{
	public:
		explicit MiniMapInterface(const std::string& name); //442340.
		virtual ~MiniMapInterface(); //442290; no explicit mapWindow delete.
		virtual int OnEvent(::GUI::Window*,::GUI::Event*); //4420A0, adjustment442300.
		virtual void BeforeUpdate(); //4429D0, adjustment442B20.
		//BeforeDraw shared404B70. OnCursor shared439610 via4422F0 adjustment.
		void Refresh(); //442B30, current Map is rebuilt before cache publication.
		void CreateMapWindow(JE::Map* map); //442710, actualRET4.
		void RefreshBackground(); //442450.
		BYTE RefreshExploredMask(); //441D20, always returns0 in AL.
		void CenterOnSelection(); //441D90.
		void SetZoom(LONG index,BYTE anchored,LONG anchorX,LONG anchorY); //441EE0, actualRET16.

		::AreaMapWindow* mapWindow; //+04, actual child of authored winMiniMap.
		LONG zoomIndex; //+08, ctor2.
		DWORD cachedMapID; //+0C, ctor0.
		std::string cachedOctreeName; //+10; Map.octreeName(+2C), not resourceName.
		BYTE flag2C,flag2D; //Ctor0; no observed use in this family.
	};
}

