#pragma once

#include <windows.h>
#include <d3d8.h>
#include <list>
#include <set>

class Gfx_Texture;
namespace JE { class World; class AreaMap; struct AreaMapBits; }

struct MapMaskCell
{
	DWORD mapID;
	LONG column, row;
	BYTE value; //598330/598390 write only this byte; three padding bytes stay unwritten.
};

// Actual global RTTI MapMaskManager/table69F918. Normal containers own nodes;
// texture pointers are borrowed, while exploredTexture is a graphics handle.
class MapMaskManager
{
public:
	MapMaskManager(); //598560, no arguments; original0x44 owner.
	virtual ~MapMaskManager(); //598400, deleting wrapper598610.
	void Clear(); //597B60, called by Graphics581330.
	void Update(); //5990E0; caller discards incidental AL.
	void Update(JE::World& world); //Explicit actual-World input, same branch policy.
	void QueueExploredChange(DWORD mapID, LONG column, LONG row, BYTE value); //598330.
	void QueueVisibleChange(DWORD entityID, DWORD mapID, LONG column, LONG row, BYTE value); //598390.
	BYTE UpdateExploredMask(DWORD mapID); //5979D0, existing MiniMap consumer.
	D3DFORMAT FindExploredMaskFormat(); //596A30, actual D3D8 capability probe.
	DWORD CreateExploredMaskTexture(LONG width, LONG height, D3DFORMAT format); //5978C0.

	BYTE CopyExplored(Gfx_Texture* texture, const JE::AreaMap* area); //597920, thiscall/RET8.
	void DrawExplored(Gfx_Texture* texture, const D3DLOCKED_RECT& lock, D3DFORMAT format,
		LONG column, LONG row, const JE::AreaMap& area); //596BC0, thiscall/RET24.

	DWORD cachedMapID; //+04.
	Gfx_Texture* automapMask; //+08.
	Gfx_Texture* visibleMask; //+0C.
	std::list<MapMaskCell> exploredChanges, visibleChanges; //+10/+1C,16-byte POD payloads.
	std::set<DWORD> changedEntities; //+28, node nil byte+11.
	DWORD exploredTexture; //+34, owning graphics texture handle.
	D3DFORMAT exploredFormat; //+38, constructor0.
	DWORD exploredMapID; //+3C.
	BYTE exploredEnabled; //+40.

private:
	BYTE UpdateVisible(); //598630.
	BYTE CellRectangle(const JE::AreaMap* area, Gfx_Texture* texture, LONG column, LONG row, RECT& output); //596890.
	void DrawVisible(Gfx_Texture* texture, const D3DLOCKED_RECT& lock, D3DFORMAT format,
		LONG column, LONG row, const JE::AreaMapBits& bits, LONG width, LONG height); //5972E0.
	MapMaskManager(const MapMaskManager&);
	MapMaskManager& operator=(const MapMaskManager&);
};
