#include "MapMaskManager.hpp"
#include "SystemInterface.hpp"
#include "Texture.hpp"
#include "MapMaskRaster.hpp"
#include "D3DState.hpp"
#include "entity/World.hpp"
#include "entity/Map.hpp"
#include "entity/MapFog.hpp"
#include "entity/WorldCollections.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <map>
#include <string.h>
#include <stdexcept>

namespace
{
	typedef std::map<LONG, MapMaskCell> Cells;
	void Expand(Cells& output, const MapMaskCell& cell, LONG width, LONG height)
	{
		const LONG offsets[9][2] = {{0,0},{1,0},{0,1},{1,1},{-1,0},{0,-1},{-1,-1},{1,-1},{-1,1}};
		for (DWORD i = 0; i < 9; ++i)
		{
			MapMaskCell value = cell;
			value.column += offsets[i][0]; value.row += offsets[i][1];
			if (value.column >= 0 && value.column < width && value.row >= 0 && value.row < height)
				output[value.column + width * value.row] = value;
		}
	}
	void DrawChanges(MapMaskManager* manager, Gfx_Texture* texture, const Cells& cells, const JE::AreaMap& area)
	{
		D3DFORMAT format = texture->frameCount > 0 ? texture->frames[0].format : D3DFMT_UNKNOWN;
		D3DLOCKED_RECT lock;
		if (texture->LockRect(0, &lock, NULL, 0, 0) == S_OK)
		{
			for (Cells::const_iterator i = cells.begin(); i != cells.end(); ++i)
				manager->DrawExplored(texture, lock, format, i->second.column, i->second.row, area);
			texture->UnlockRect(0);
		}
	}
}

MapMaskManager::MapMaskManager()
	: cachedMapID(0), automapMask(NULL), visibleMask(NULL), exploredTexture(0),
	  exploredFormat(D3DFMT_UNKNOWN), exploredMapID(0), exploredEnabled(0)
{
}
MapMaskManager::~MapMaskManager() //598400; actual handle release precedes reverse member lifetime.
{
	if (exploredTexture) g_GfxSystem->DeleteTexture(exploredTexture);
}
void MapMaskManager::Clear() //597B60.
{
	if (exploredTexture)
	{
		g_GfxSystem->DeleteTexture(exploredTexture);
		exploredTexture = 0;
	}
	cachedMapID = 0;
	automapMask = NULL;
	visibleMask = NULL;
	exploredFormat = D3DFMT_UNKNOWN;
	exploredMapID = 0;
	exploredEnabled = 0;
	exploredChanges.clear();
	visibleChanges.clear();
	changedEntities.clear();
}
void MapMaskManager::Update()
{
	if (!JE::g_World) throw std::logic_error("MapMaskManager599121: actual World required");
	Update(*JE::g_World);
}
void MapMaskManager::Update(JE::World& world) //5990E0.
{
	DWORD mapID = world.valueE0;
	if (!mapID) return;
	JE::Map* map = world.GetMap(mapID);
	if (!map || !map->areaMap) return;
	Gfx_Texture* texture = g_GfxSystem->AutomapMaskTexture(); //Actual+194.
	if (texture)
	{
		if (mapID == cachedMapID && texture == automapMask)
		{
			JE::AreaMap& area = *map->areaMap;
			Cells cells;
			for (std::list<MapMaskCell>::iterator i = exploredChanges.begin(); i != exploredChanges.end(); ++i)
				if (i->mapID == mapID && i->column >= 0 && i->column < area.width && i->row >= 0 && i->row < area.height)
					Expand(cells, *i, area.width, area.height);
			if (!cells.empty())
			{
				DrawChanges(this, texture, cells, area);
				if (exploredEnabled && exploredTexture)
					DrawChanges(this, g_GfxSystem->LookupTexture(exploredTexture), cells, area); //Original unchecked lookup.
			}
			exploredChanges.clear();
		}
		else
		{
			//Original rereads current map/texture before full597920 copy.
			mapID = world.valueE0;
			if (mapID)
			{
				JE::Map* selected = world.GetMap(mapID);
				if (selected && selected->areaMap)
				{
					JE::AreaMap* selectedArea = selected->areaMap;
					Gfx_Texture* selectedTexture = g_GfxSystem->AutomapMaskTexture();
					if (selectedTexture)
					{
						if (CopyExplored(selectedTexture, selectedArea))
						{
							cachedMapID = mapID;
							exploredChanges.clear();
						}
					}
				}
				if (exploredEnabled && exploredTexture)
					UpdateExploredMask(mapID);
			}
		}
		automapMask = texture; //Original publishes initial texture, not the reread result.
	}
	//Even a NULL automap mask continues into the independent visibility texture.
	UpdateVisible();
}
BYTE MapMaskManager::UpdateVisible() //598630.
{
	JE::IWorldClient* client = JE::g_World;
	const DWORD mapID = client->GetCurrentMapID();
	if (!mapID) return 0;
	JE::IWorld* world = JE::g_World;
	JE::Map* map = world->GetMap(mapID);
	if (!map || !map->areaMap) return 0;
	JE::AreaMap* area = map->areaMap;
	Gfx_Texture* texture = g_GfxSystem->VisibleMaskTexture(); //Actual+198.
	if (!texture) return 0;
	BYTE changed = texture != visibleMask;
	visibleMask = texture; //Original publishes before dimensions/dirty decisions.
	const LONG width = area->width;
	const LONG height = area->height;
	if (width > 0 && height > 0
		&& (changed || (!visibleChanges.empty() && !changedEntities.empty())))
	{
		area->visible.Clear(); //5AD7B0 precedes the fresh party lookup.
		std::list<JE::Entity*> prepare;
		JE::Party* party = JE::GetWorldInstance()->GetParty();
		for (std::vector<JE::Entity*>::iterator i = party->members.begin(); i != party->members.end(); ++i)
		{
			JE::Entity* entity = *i;
			if (changed || changedEntities.find(entity->entityID) == changedEntities.end())
			{
				prepare.push_back(entity);
				changedEntities.insert(entity->entityID);
			}
		}
		for (std::list<JE::Entity*>::iterator i = prepare.begin(); i != prepare.end(); ++i)
			(*i)->QueueCurrentVisibility(); //5987E3, actual visible-only producer549710.
		JE::AreaMapBits bits;
		Cells visibleCells, cells;
		for (std::list<MapMaskCell>::iterator i = visibleChanges.begin(); i != visibleChanges.end(); ++i)
		{
			const MapMaskCell cell = *i;
			if (cell.mapID == mapID && cell.column >= 0 && cell.column < width && cell.row >= 0 && cell.row < height)
			{
				const LONG index = LONG(DWORD(cell.column) + DWORD(width) * DWORD(cell.row));
				bits.SetBit(index, cell.value);
				visibleCells[index] = cell;
				if (cell.value) area->SetVisible(cell.column, cell.row, 1);
			}
		}
		for (Cells::iterator i = visibleCells.begin(); i != visibleCells.end(); ++i)
			Expand(cells, i->second, width, height);
		D3DLOCKED_RECT lock;
		if (!cells.empty() && texture->LockRect(0, &lock, NULL, 0, 0) == S_OK)
		{
			DWORD height = texture->frameCount > 0 ? texture->frames[0].height : 0;
			D3DFORMAT format = texture->frameCount > 0 ? texture->frames[0].format : D3DFMT_UNKNOWN;
			memset(lock.pBits, 0, lock.Pitch * height);
			for (Cells::iterator i = cells.begin(); i != cells.end(); ++i)
				DrawVisible(texture, lock, format, i->second.column, i->second.row, bits, width, height);
			texture->UnlockRect(0);
		}
		visibleChanges.clear();
		changedEntities.clear();
	}
	return 1;
}

void MapMaskManager::QueueExploredChange(DWORD mapID, LONG column, LONG row, BYTE value) //598330.
{
	MapMaskCell cell;
	cell.mapID = mapID; cell.column = column; cell.row = row; cell.value = value;
	exploredChanges.push_back(cell);
}
void MapMaskManager::QueueVisibleChange(DWORD entityID, DWORD mapID, LONG column, LONG row, BYTE value) //598390.
{
	MapMaskCell cell;
	cell.mapID = mapID; cell.column = column; cell.row = row; cell.value = value;
	visibleChanges.push_back(cell);
	changedEntities.insert(entityID);
}
D3DFORMAT MapMaskManager::FindExploredMaskFormat() //596A30.
{
	D3DFORMAT formats[4] = {D3DFMT_A8R3G3B2, D3DFMT_A4R4G4B4,
		D3DFMT_A1R5G5B5, D3DFMT_A8R8G8B8};
	for (DWORD index = 0; index < 4; ++index)
		if (SUCCEEDED(g_D3D.direct3D->CheckDeviceFormat(g_D3D.currentAdapter,
			g_D3D.currentDevice->type, g_D3D.presentation.BackBufferFormat,
			D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, formats[index])))
			return formats[index];
	JE::FatalError("MapMaskManager::FindExploredMaskFormat - failed to find a suitable render target format for explored mask!\n");
	return D3DFMT_UNKNOWN;
}

DWORD MapMaskManager::CreateExploredMaskTexture(LONG width, LONG height, D3DFORMAT format) //5978C0.
{
	DWORD handle = 0;
	g_GfxSystem->CreateAndAddTexture(&handle, width, height, 1, format, D3DPOOL_MANAGED);
	if (handle)
	{
		Gfx_Texture* texture = g_GfxSystem->LookupTexture(handle);
		if (texture) texture->Clear();
	}
	return handle;
}
BYTE MapMaskManager::UpdateExploredMask(DWORD mapID) //5979D0.
{
	if (exploredMapID != mapID && exploredTexture)
	{
		if (g_GfxSystem->DeleteTexture(exploredTexture))
			JE::LogDebugString("MapMaskManager::UpdateExploredMask - ExploredMask still in use!!");
		exploredTexture = 0;
	}
	JE::IWorld* world = JE::g_World;
	JE::Map* map = world->GetMap(mapID);
	if (!map) return 0;
	JE::AreaMap* area = map->areaMap;
	if (!area)
	{
failed:
		return 0;
	}
	if (exploredMapID != mapID)
	{
		const LONG width = area->screenWidth;
		const LONG height = area->screenHeight;
		exploredFormat = FindExploredMaskFormat();
		exploredTexture = CreateExploredMaskTexture(width, height, exploredFormat);
		exploredMapID = mapID;
	}
	if (!exploredTexture) goto failed;
	Gfx_Texture* texture = g_GfxSystem->LookupTexture(exploredTexture);
	if (!texture) goto failed;
	return CopyExplored(texture, area);
}

BYTE MapMaskManager::CopyExplored(Gfx_Texture* texture, const JE::AreaMap* area)
	{
		if (!area || !texture) return 0;
		const LONG width = area->width, height = area->height;
		if (width > 0 && height > 0)
		{
			D3DFORMAT format = texture->frameCount > 0 ? texture->frames[0].format : D3DFMT_UNKNOWN;
			D3DLOCKED_RECT lock;
			if (texture->LockRect(0, &lock, NULL, 0, 0) == S_OK)
			{
				for (LONG y = 0; y < height; ++y)
					for (LONG x = 0; x < width; ++x) DrawExplored(texture, lock, format, x, y, *area);
				texture->UnlockRect(0);
			}
		}
		return 1; //Original returns1 even when LockRect failed.
	}
