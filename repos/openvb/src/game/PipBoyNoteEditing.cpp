// Implementation fragment: compiled once by PipBoyCompilation.cpp.
#include <winsock2.h>
#include "game/PipBoyInterface.hpp"
#include "game/PipBoyAddNoteInterface.hpp"
#include "game/AreaMapWindow.hpp"
#include "game/AreaMapIcon.hpp"
#include "game/GamePlayer.hpp"
#include "game/AreaMapResources.hpp"
#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "gui/SkinServices.hpp"
#include "graphics/G3DModel.hpp"
#include "graphics/G3DTextureImage.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/MapMaskManager.hpp"
#include "graphics/MapMaskRaster.hpp"
#include "graphics/StringFormat.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/TextureResource.hpp"
#include "graphics/Texture.hpp"
#include "network/EngineClient.hpp"
#include "startup/EngineStringSystem.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include <climits>
#include <cstring>
#include <set>

namespace
{
	static const char* MapLabelName(LONG column, LONG row) //44A370, private ECX-column/EAX-row convention.
	{
		if (column < 0 || column >= 2 || row < 0 || row >= 14) return NULL;
		return MapLabelNames[row][column];
	}
	void FitMapLabel(::GUI::Label* label)
	{
		LONG width,height,left,right,top,bottom;
		label->GetSize(width,height);
		label->GetPadding(left,right,top,bottom);
		label->Resize(left+label->GetMaximumRowWidth(),height);
	}
}

namespace F3
{
	static LONG MapGroupIndex(const std::string& name, LONG top) //44A940.
	{
		LONG row = 0;
		while (_stricmp(name.c_str(), MapLabelNames[row][0]))
			if (++row >= 14) return -1;
		if (row + top <= 0) return -1;
		return row + top - 1;
	}

	static LONG MapEntryIndex(const std::string& name, LONG top, LONG count) //44A980.
	{
		LONG row = 0;
		while (_stricmp(name.c_str(), MapLabelNames[row][1]))
			if (++row >= 14) return -1;
		const LONG selected = row + top;
		if (count <= 0 || selected < 7 || selected >= count + 7) return -1;
		return selected - 7;
	}

	static LONG MapModeIndex(const std::string& name, LONG top) //44A9E0.
	{
		LONG row = 0;
		while (_stricmp(name.c_str(), MapLabelNames[row][1]))
			if (++row >= 14) return 0;
		const LONG selected = row + top;
		if (selected < 0 || selected > 4) return 0;
		return selected + 1;
	}

	std::string PipBoyInterface::SelectedMapTextureName() //44D960.
	{
		std::string result;
		JE::AreaMap* map = SelectedAreaMap();
		if (map) result = map->textureName;
		return result;
	}

	void PipBoyInterface::AllocateMapMask() //44AB80.
	{
		if (cachedTexture) { g_GfxSystem->DeleteTexture(cachedTexture); cachedTexture = 0; }
		MapMaskManager* masks = static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x74->maskManager;
		JE::IWorldClient* world = JE::g_World;
		JE::Map* activeMap = world->GetCurrentMap();
		JE::AreaMap* selected = SelectedAreaMap();
		DWORD texture = 0;
		if (selected)
		{
			if (activeMap && activeMap->areaMap == selected) texture = masks->exploredTexture;
			else
			{
				const LONG width = selected->screenWidth;
				const LONG height = selected->screenHeight;
				const D3DFORMAT format = masks->FindExploredMaskFormat();
				cachedMaskFormat = format;
				cachedTexture = masks->CreateExploredMaskTexture(width, height, format);
				texture = cachedTexture;
			}
		}
		AreaMapWindow* window = primaryMap;
		if (window) { window->maskTexture = texture; window->dirty = 1; }
		window = secondaryMap;
		if (window) { window->maskTexture = texture; window->dirty = 1; }
	}

	BYTE PipBoyInterface::RefreshMapMask() //44AC50.
	{
		MapMaskManager* masks = static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x74->maskManager;
		JE::IWorldClient* world = JE::g_World;
		JE::Map* activeMap = world->GetCurrentMap();
		JE::AreaMap* selected = SelectedAreaMap();
		if (!selected) return 0;
		if (activeMap && activeMap->areaMap == selected)
		{
			if (!masks->UpdateExploredMask(JE::GetCurrentClientMapID())) return 0;
			AreaMapWindow* window = primaryMap;
			const DWORD texture = masks->exploredTexture;
			window->maskTexture = texture; window->dirty = 1;
			window = secondaryMap;
			window->maskTexture = texture; window->dirty = 1;
			return 1;
		}
		Gfx_Texture* target = g_GfxSystem->LookupTexture(cachedTexture);
		if (!target || !masks->CopyExplored(target, selected)) return 0;
		AreaMapWindow* window = primaryMap;
		window->maskTexture = cachedTexture; window->dirty = 1;
		window = secondaryMap;
		window->maskTexture = cachedTexture; window->dirty = 1;
		return 1;
	}

	void PipBoyInterface::SetupMapWindows() //44EF20.
	{
		JE::AreaMap* selected = SelectedAreaMap();
		const float zoom = g_PipBoyMapZooms[mapZoomIndex];
		if (primaryMap) { RemoveWindow(primaryMap); delete primaryMap; }
		primaryMap = new AreaMapWindow(selected);
		primaryMap->ApplySkin("AreaMap.ini");
		if (secondaryMap) { RemoveWindow(secondaryMap); delete secondaryMap; }
		secondaryMap = new AreaMapWindow(selected);
		secondaryMap->ApplySkin("AreaMap.ini");
		::GUI::Window* parent = FindWindow("winMap1");
		LONG x,y,width,height;
		if (parent)
		{
			parent->GetAbsolutePosition(x,y); parent->GetSize(width,height);
			primaryMap->SetAbsolutePosition(x,y); primaryMap->Resize(width,height);
		}
		primaryMap->SetName("AreaMapWindow1"); parent->AddChild(primaryMap);
		primaryMap->RebuildGeometry(); primaryMap->zoom = zoom; primaryMap->dirty = 1;
		std::string textureName = SelectedMapTextureName();
		DWORD texture = 0; width = height = 0;
		if (!textureName.empty()) texture = Gfx_RequestTextureResource(textureName.c_str(),1,1,5,0);
		if (texture)
		{
			JE::Resources::ResourceData* resource = ::GUI::SkinServices::Demand(textureName.c_str());
			if (resource)
			{
				G3D_Texture image; image.width = DWORD(-1); image.height = 0;
				if (G3D_ReadTga(image,::GUI::SkinServices::Data(resource)) || LONG(image.width) > 0)
				{ width = LONG(image.width); height = LONG(image.height); }
				::GUI::SkinServices::Release(resource);
			}
			primaryMap->SetTexture(texture,0,width,height,0,0,-1,-1,1,0);
			primaryMap->SetTileMode(4);
		}
		else primaryMap->LoadTexture("AreaMap-middle.bmp",0,0,0,-1,-1,1);
		primaryMap->flagF8D = 0; primaryMap->dirty = 1;
		parent = FindWindow("winMap2");
		if (parent)
		{
			LONG secondWidth,secondHeight;
			parent->GetAbsolutePosition(x,y); parent->GetSize(secondWidth,secondHeight);
			secondaryMap->SetAbsolutePosition(x,y); secondaryMap->Resize(secondWidth,secondHeight);
		}
		secondaryMap->SetName("AreaMapWindow2"); parent->AddChild(secondaryMap);
		secondaryMap->RebuildGeometry(); secondaryMap->zoom = zoom; secondaryMap->dirty = 1;
		if (texture)
		{
			secondaryMap->SetTexture(texture,0,width,height,0,0,-1,-1,1,1);
			secondaryMap->SetTileMode(4);
		}
		else secondaryMap->LoadTexture("AreaMap-middle.bmp",0,0,0,-1,-1,1);
		secondaryMap->flagF8D = 0; secondaryMap->dirty = 1;
		AllocateMapMask(); RefreshMapMask(); CenterMap(); ClampMapOrigin(1,0,0);
		static const char* front[8] = {"lblMapMode","lblAddNote","lblEditNote","lblDeleteNote",
			"lblZoomIn","lblZoomOut","btnLeft","btnRight"};
		for (LONG index = 0; index < 8; ++index) FindWindow(front[index])->BringToFront();
		primaryMap->SetNoteHoverEnabled(1); secondaryMap->SetNoteHoverEnabled(1);
		primaryMap->SetPartyHoverEnabled(1); secondaryMap->SetPartyHoverEnabled(1);
		JE::Map* activeMap = JE::g_World->GetMap(JE::g_World->valueE0);
		const BYTE show = activeMap && selected && activeMap->areaMap == selected;
		primaryMap->showParty = show; primaryMap->dirty = 1;
		secondaryMap->showParty = show; secondaryMap->dirty = 1;
		primaryMap->showCamera = show; primaryMap->dirty = 1;
		secondaryMap->showCamera = show; secondaryMap->dirty = 1;
		cachedMapID = activeMap ? activeMap->idB4 : 0;
		cachedMapName = activeMap ? activeMap->octreeName : "";
		ShowLilPip(selected == NULL,mapMode);
	}

	void PipBoyInterface::SetNoteText(const std::string& text) //44D900.
	{
		JE::AreaMap* map = SelectedAreaMap();
		if (map && !JE::IsStringBlank(text.c_str(), " \n\t\r") && noteIndex >= 0)
		{
			JE::AreaMapNote* target = map->FindNote(noteIndex);
			if (target) target->text = text;
		}
	}

	BYTE PipBoyInterface::FinishNoteEdit(BYTE accepted, BYTE remove) //4533F0.
	{
		const LONG index = noteIndex;
		if (index >= 0)
		{
			bool empty = true;
			JE::AreaMap* map = SelectedAreaMap();
			if (map)
			{
				JE::AreaMapNote* target = map->FindNote(index);
				if (target) empty = target->text.empty();
			}
			if (!accepted)
			{
				if (empty || remove)
				{
					JE::AreaMap* currentMap = SelectedAreaMap();
					if (currentMap) currentMap->RemoveNote(index);
				}
				else if (map)
				{
					JE::AreaMapNote* target = map->FindNote(index);
					if (target)
					{
						*target = note;
						map->changes |= 2;
					}
				}
			}
		}
		noteIndex = -1;
		if (accepted) noteIconIndex = addNote->iconIndex;
		SetNoteEditMode(0);
		::GUI::MouseState mouse;
		::GUI::g_Manager.GetMouseState(mouse);
		if (secondaryMap)
		{
			secondaryMap->RebuildVisibleSamples();
			secondaryMap->UpdatePartyHover(mouse.x,mouse.y);
			secondaryMap->UpdateNoteHover(mouse.x,mouse.y);
		}
		if (mapMode && primaryMap)
		{
			primaryMap->RebuildVisibleSamples();
			primaryMap->UpdatePartyHover(mouse.x,mouse.y);
			primaryMap->UpdateNoteHover(mouse.x,mouse.y);
		}
		return UpdateNotePopup(mouse.x,mouse.y);
	}

	void PipBoyInterface::SetNoteEditMode(DWORD mode) //452240.
	{
		if (valueBC==1)
		{
			const LONG index = noteIndex;
			if (index >= 0)
			{
				JE::AreaMap* map = SelectedAreaMap();
				if (map) map->RemoveNote(index);
				noteIndex = -1;
			}
		}
		valueBC = LONG(mode);
		if (mode==1)
		{
			JE::AreaMap* map = SelectedAreaMap();
			if (!map) return;
			primaryMap->SetNoteHoverEnabled(0);
			secondaryMap->SetNoteHoverEnabled(0);
			primaryMap->SetPartyHoverEnabled(0);
			secondaryMap->SetPartyHoverEnabled(0);
			const DWORD handle = addNote->GetIconTexture(noteIconIndex);
			if (handle)
			{
				Gfx_Texture* texture = g_GfxSystem->LookupTexture(handle);
				if (texture)
				{
					const LONG width = texture->frameCount>0 ? LONG(texture->frames[0].width) : 0;
					const LONG height = texture->frameCount>0 ? LONG(texture->frames[0].height) : 0;
					std::string name = addNote->GetIconTextureName(noteIconIndex);
					const float position[3] = {0,0,0};
					noteIndex = map->AddNote(name,width/2,height/2,width,height,std::string(),position,0);
					JE::AreaMapNote* target = map->FindNote(noteIndex);
					if (target) target->flag5D = 1;
				}
			}
		}
		else
		{
			primaryMap->SetNoteHoverEnabled(1);
			secondaryMap->SetNoteHoverEnabled(1);
			if (mode-2 < 2)
			{
				primaryMap->SetPartyHoverEnabled(0);
				secondaryMap->SetPartyHoverEnabled(0);
			}
			else
			{
				primaryMap->SetPartyHoverEnabled(1);
				secondaryMap->SetPartyHoverEnabled(1);
			}
		}
		RefreshMaps();
	}

	BYTE PipBoyInterface::UpdateNotePopup(LONG x, LONG y) //4524B0.
	{
		BYTE shown = 0;
		if (valueBC==1 || !secondaryMap || !secondaryMap->Contains(x,y) ||
			!(shown = ConfigureNotePopup(secondaryMap,x,y)))
		{
			if (valueBC==1 || !mapMode || !primaryMap || !primaryMap->Contains(x,y) ||
				!(shown = ConfigureNotePopup(primaryMap,x,y)))
			{
				if (addNote) addNote->SetHidden(1);
			}
		}
		return shown;
	}

	BYTE PipBoyInterface::ConfigureNotePopup(AreaMapWindow* window, LONG x, LONG y) //450080.
	{
		float scaleX,scaleY;
		::GUI::g_Manager.GetScale(scaleX,scaleY);
		x = LONG(static_cast<LONGLONG>(float(x)*scaleX));
		y = LONG(static_cast<LONGLONG>(float(y)*scaleY));
		std::list<AreaMapIcon*>::iterator i = window->noteIcons.begin();
		const std::list<AreaMapIcon*>::iterator initialEnd = window->noteIcons.end();
		if (i==initialEnd) return 0;
		do
		{
			AreaMapIcon* icon = *i;
			LONG left,top,width,height;
			icon->GetRect(left,top,width,height);
			if (x>=left && x<left+width && y>=top && y<top+height)
			{
				JE::AreaMap* map = SelectedAreaMap();
				JE::AreaMapNote* target = icon->GetNote();
				addNote->SetPopupPosition(LONG(static_cast<LONGLONG>(double(left+width)/scaleX)),
					LONG(static_cast<LONGLONG>(double(top+height)/scaleY)));
				addNote->Open(this,map,target,1);
				return 1;
			}
		} while (++i!=window->noteIcons.end());
		return 0;
	}

	void PipBoyInterface::RefreshMaps() //450230, real player map cache and authored labels.
	{
		valueB8 = 0;
		valueB4 = 0;
		areaIDs.clear();
		areaMaps.clear();
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		GamePlayer* player = dynamic_cast<GamePlayer*>(JE::GetEntityPtr(client->field0xE4));
		if (player)
		{
			PlayerAreaMaps* cache = player->areaMaps;
			std::set<DWORD> areas;
			for (std::map<std::string,JE::AreaMap*>::const_iterator i=cache->maps.begin();i!=cache->maps.end();++i)
				areas.insert(i->second->value4);
			for (std::set<DWORD>::const_iterator i=areas.begin();i!=areas.end();++i) areaIDs.push_back(*i);
			DWORD selectedArea = DWORD(-1);
			if (valueA4>=0 && valueA4<LONG(areaIDs.size())) selectedArea = areaIDs[valueA4];
			for (std::map<std::string,JE::AreaMap*>::const_iterator i=cache->maps.begin();i!=cache->maps.end();++i)
				if (i->second->value4==selectedArea) areaMaps.push_back(i->second);
			valueB8 = LONG(areaIDs.size());
			valueB4 = LONG(areaMaps.size());
		}
		if (mapMode)
		{
			::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow("lblAddNote"));
			if (valueBC==1)
			{
				label->SetTextColor(0xFF000080);
				label->SetValue3B(0);
			}
			else if (valueA8>=0)
			{
				label->SetTextColor(mapsColor);
				label->SetValue3B(0);
			}
			else
			{
				label->SetTextColor(0x40808080);
				label->SetValue3B(1);
			}

			label = static_cast< ::GUI::Label*>(FindWindow("lblEditNote"));
			if (valueBC==2)
			{
				label->SetTextColor(0xFF000080);
				label->SetValue3B(0);
			}
			else if (valueA8>=0)
			{
				label->SetTextColor(mapsColor);
				label->SetValue3B(0);
			}
			else
			{
				label->SetTextColor(0x40808080);
				label->SetValue3B(1);
			}

			label = static_cast< ::GUI::Label*>(FindWindow("lblDeleteNote"));
			if (valueBC==3)
			{
				label->SetTextColor(0xFF000080);
				label->SetValue3B(0);
			}
			else if (valueA8>=0)
			{
				label->SetTextColor(mapsColor);
				label->SetValue3B(0);
			}
			else
			{
				label->SetTextColor(0x40808080);
				label->SetValue3B(1);
			}

			label = static_cast< ::GUI::Label*>(FindWindow("lblZoomIn"));
			if (valueA8>=0 && mapZoomIndex>0)
			{
				label->SetTextColor(mapsColor);
				label->SetValue3B(0);
			}
			else
			{
				label->SetTextColor(0x40808080);
				label->SetValue3B(1);
			}

			label = static_cast< ::GUI::Label*>(FindWindow("lblZoomOut"));
			if (valueA8>=0 && mapZoomIndex<4)
			{
				label->SetTextColor(mapsColor);
				label->SetValue3B(0);
			}
			else
			{
				label->SetTextColor(0x40808080);
				label->SetValue3B(1);
			}
			return;
		}
		for (LONG row=0;row<14;++row)
		{
			const LONG index = row+mapTop;
			::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(MapLabelName(0,row)));
			if (!label) continue;
			LONG left,right,top,bottom;
			label->GetPadding(left,right,top,bottom);
			if (index==0)
			{
				label->SetStringReference(408);
				label->SetValue3B(1);
				label->SetHidden(0);
				label->SetPadding(alarmEmptyMetric,right,top,bottom);
			}
			else if (index<=valueB8)
			{
				std::string text;
				JE::FormatString(text,"Area %d",areaIDs[index-1]);
				label->SetText(text);
				label->SetValue3B(0);
				label->SetHidden(0);
				if (valueA4==index-1)
				{
					text = std::string()+text; //Original empty shared string concatenation; no fabricated markup.
					label->SetTextColor(0xFF000080);
					label->SetText(text);
					label->SetPadding(0,right,top,bottom);
				}
				else
				{
					label->SetTextColor(mapsColor);
					label->SetText(text);
					label->SetPadding(alarmEmptyMetric,right,top,bottom);
				}
			}
			else label->SetHidden(1);
			FitMapLabel(label);
		}
		for (LONG row=0;row<14;++row)
		{
			const LONG index = row+mapTop;
			::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow(MapLabelName(1,row)));
			if (!label) continue;
			LONG left,right,top,bottom,width,height;
			label->GetPadding(left,right,top,bottom);
			label->GetSize(width,height);
			label->Resize(1024,height);
			if (index>=0 && index<3)
			{
				std::string text;
				F3::GetString(409+index,text);
				if (valueBC==index+1)
				{
					text = std::string()+text;
					label->SetTextColor(0xFF000080);
					label->SetText(text);
					label->SetPadding(0,right,top,bottom);
					label->SetValue3B(0);
				}
				else
				{
					label->SetTextColor(valueA8<0 ? 0x40808080 : mapsColor);
					label->SetValue3B(valueA8<0);
					label->SetText(text);
					label->SetPadding(alarmEmptyMetric,right,top,bottom);
				}
				label->SetHidden(0);
			}
			else if (index==3 || index==4)
			{
				label->SetStringReference(409+index);
				label->SetHidden(0);
				label->SetPadding(alarmEmptyMetric,right,top,bottom);
				const bool enabled = valueA8>=0 && (index==3 ? mapZoomIndex>0 : mapZoomIndex<4);
				label->SetTextColor(enabled ? mapsColor : 0x40808080);
				label->SetValue3B(!enabled);
			}
			else if (valueB4>0 && index==6)
			{
				label->SetStringReference(414);
				label->SetValue3B(1);
				label->SetHidden(0);
				label->SetPadding(alarmEmptyMetric,right,top,bottom);
				label->SetTextColor(mapsColor);
			}
			else if (valueB4>0 && index>=7 && index<valueB4+7)
			{
				const LONG mapIndex = index-7;
				JE::AreaMap* map = areaMaps[mapIndex];
				label->SetValue3B(0);
				label->SetHidden(0);
				std::string text;
				F3::GetString(map->value8,text);
				if (text.empty()) JE::FormatString(text,"<unnamed map %d>",mapIndex+1);
				if (valueA8==mapIndex)
				{
					text = std::string()+text;
					label->SetTextColor(0xFF000080);
					label->SetText(text);
					label->SetPadding(0,right,top,bottom);
				}
				else
				{
					label->SetTextColor(mapsColor);
					label->SetText(text);
					label->SetPadding(alarmEmptyMetric,right,top,bottom);
				}
			}
			else label->SetHidden(1);
			FitMapLabel(label);
		}
	}
}
