// Implementation fragment: compiled once by PipBoyCompilation.cpp.
#include <winsock2.h>
#include "game/PipBoyInterface.hpp"
#include "game/PipBoyAddNoteInterface.hpp"
#include "game/AreaMapWindow.hpp"
#include "game/AreaMapIcon.hpp"
#include "gui/Button.hpp"
#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "entity/Entity.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "graphics/Octree.hpp"
#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include <mmsystem.h>
#include <cstring>

namespace F3
{
	static LONG AlarmNameIndex(const std::string& name); //44A840, single actual owner in PipBoyInterface.cpp.
	static LONG TaskGroupIndex(const std::string&, LONG top); //44A880.
	static LONG TaskEntryIndex(const std::string&, LONG top, LONG openCount, LONG closedCount); //44A8C0.
	static LONG MapGroupIndex(const std::string&, LONG top); //44A940.
	static LONG MapEntryIndex(const std::string&, LONG top, LONG count); //44A980.
	static LONG MapModeIndex(const std::string&, LONG top); //44A9E0.
	static LONG DownloadHolodiscIndex(const std::string&, LONG top); //44AA30.
	static LONG DownloadVideoIndex(const std::string&, LONG top, LONG count); //44AA70.

	void PipBoyInterface::HandleButtonInput(::GUI::Window* window, LONG eventType) //451B80.
	{
		if (window->type != 1) return;
		::GUI::Button* button = static_cast< ::GUI::Button*>(window);
		if (!button->IsEnabled()) return;
		DWORD now;
		if (eventType)
		{
			if (eventType != 4 && eventType != 7) return;
			now = timeGetTime();
			value1BC = now;
		}
		else
		{
			if (button->GetState() != 2) return;
			now = timeGetTime();
			if (!value1BC || value1BC + holdDelay >= now) return;
			if (!value1C0 || value1C0 + repeatDelay >= now) return;
		}
		value1C0 = now;
		if (currentPage == 2)
		{
			if (!_stricmp(button->GetName(), "btnUp1"))
			{
				if (taskTop <= 0) return;
				LONG top = --taskTop;
				const LONG rows = GetTaskRowCount();
				if (rows - top <= 0)
				{
					do { --top; } while (rows - top <= 0);
					taskTop = top;
				}
				RefreshTaskLabels();
			}
			else if (!_stricmp(button->GetName(), "btnDown1"))
			{
				const LONG rows = GetTaskRowCount();
				if (rows <= 14 || taskTop >= rows - 14) return;
				++taskTop;
				RefreshTaskLabels();
			}
			else if (!_stricmp(button->GetName(), "btnUp2"))
			{
				::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow("lblTasks2"));
				LONG horizontal, vertical;
				label->GetScroll(horizontal, vertical);
				if (vertical <= 0) return;
				float scaleX, scaleY;
				::GUI::g_Manager.GetScale(scaleX, scaleY);
				vertical -= LONG(static_cast<LONGLONG>(label->GetFontSize() * scaleY));
				if (vertical < 0) vertical = 0;
				label->SetScroll(horizontal, vertical);
				taskScroll = vertical;
			}
			else if (!_stricmp(button->GetName(), "btnDown2"))
			{
				::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow("lblTasks2"));
				LONG horizontal, vertical, width, height, left, right, top, bottom;
				label->GetScroll(horizontal, vertical);
				label->GetSize(width, height);
				label->GetPadding(left, right, top, bottom);
				width -= left + right;
				height -= top + bottom;
				if (width < 0) width = 0;
				if (height < 0) height = 0;
				const LONG limit = label->GetTextHeight() - height;
				if (vertical >= limit) return;
				float scaleX, scaleY;
				::GUI::g_Manager.GetScale(scaleX, scaleY);
				vertical += LONG(static_cast<LONGLONG>(label->GetFontSize() * scaleY));
				if (vertical >= limit) vertical = limit - 1;
				label->SetScroll(horizontal, vertical);
				taskScroll = vertical;
			}
			else return;
			RefreshButtons();
		}
		else if (currentPage == 3)
		{
			if (!_stricmp(button->GetName(), "btnUp1"))
			{
				if (mapMode)
				{
					mapY -= 8;
					ClampMapOrigin(1, 0, -8);
					return;
				}
				if (mapTop <= 0) return;
				LONG top = --mapTop;
				const LONG rows = GetMapRowCount();
				if (rows - top <= 0)
				{
					do { --top; } while (rows - top <= 0);
					mapTop = top;
				}
				RefreshMaps();
			}
			else if (!_stricmp(button->GetName(), "btnDown1"))
			{
				if (mapMode)
				{
					mapY += 8;
					ClampMapOrigin(1, 0, 8);
					return;
				}
				const LONG rows = GetMapRowCount();
				if (rows <= 14 || mapTop >= rows - 14) return;
				++mapTop;
				RefreshMaps();
			}
			else if (!_stricmp(button->GetName(), "btnUp2"))
			{
				mapY -= 8;
				ClampMapOrigin(1, 0, -8);
				return;
			}
			else if (!_stricmp(button->GetName(), "btnDown2"))
			{
				mapY += 8;
				ClampMapOrigin(1, 0, 8);
				return;
			}
			else if (!_stricmp(button->GetName(), "btnLeft"))
			{
				mapX -= 8;
				ClampMapOrigin(1, -8, 0);
				return;
			}
			else if (!_stricmp(button->GetName(), "btnRight"))
			{
				mapX += 8;
				ClampMapOrigin(1, 8, 0);
				return;
			}
			else return;
			RefreshButtons();
		}
		else if (currentPage == 4)
		{
			if (!_stricmp(button->GetName(), "btnUp1"))
			{
				if (downloadsTop <= 0) return;
				LONG top = --downloadsTop;
				const LONG rows = GetDownloadRowCount();
				if (rows - top <= 0)
				{
					do { --top; } while (rows - top <= 0);
					downloadsTop = top;
				}
				RefreshDownloads();
			}
			else if (!_stricmp(button->GetName(), "btnDown1"))
			{
				const LONG rows = GetDownloadRowCount();
				if (rows <= 14 || downloadsTop >= rows - 14) return;
				++downloadsTop;
				RefreshDownloads();
			}
			else return;
			RefreshButtons();
		}
	}

	void PipBoyInterface::HandleLabelInput(::GUI::Window* window, LONG /*eventType*/) //452910.
	{
		switch (currentPage)
		{
		case 1:
		{
			const LONG alarmIndex = AlarmNameIndex(window->GetName());
			if (alarmIndex != -1)
			{
				alarmText = window->GetName();
				RefreshAlarmChoices();
			}
			else if (!_stricmp(window->GetName(), "lblRest")) SelectedAlarmIndex();
			break;
		}
		case 2:
			{
				const LONG group = TaskGroupIndex(window->GetName(), taskTop);
				if (group >= 0 && value78 != group)
				{
					value78 = group;
					value7C = -1;
					taskScroll = 0;
					RefreshTaskLabels();
					RefreshTaskDescription();
					RefreshButtons();
				}
				else
				{
					const LONG task = TaskEntryIndex(window->GetName(), taskTop, LONG(value98), LONG(value9C));
					if (task >= 0)
					{
						value7C = task;
						taskScroll = 0;
						RefreshTaskLabels();
						RefreshTaskDescription();
					}
				}
			}
			break;
		case 3:
			{
				const LONG group = MapGroupIndex(window->GetName(), mapTop);
				if (group >= 0 && valueA4 != group)
				{
					valueA4 = group;
					valueA8 = -1;
					SetNoteEditMode(0);
					RefreshMaps();
					RefreshButtons();
					SetupMapWindows();
				}
				else
				{
					const LONG entry = MapEntryIndex(window->GetName(), mapTop, valueB4);
					if (entry >= 0)
					{
						if (valueA8 != entry)
						{
							SetNoteEditMode(0);
							valueA8 = entry;
							RefreshMaps();
							SetupMapWindows();
						}
					}
					else if (!_stricmp(window->GetName(), "lblMapMode")) ToggleMapMode();
					else
					{
						const LONG mode = MapModeIndex(window->GetName(), mapTop);
						if (mode && valueA8 >= 0)
						{
							switch (mode)
							{
							case 1: case 2: case 3: ToggleNoteEditMode(mode); break;
							case 4: SetMapZoom(DWORD(mapZoomIndex - 1), 0, 0, 0); break;
							case 5: SetMapZoom(DWORD(mapZoomIndex + 1), 0, 0, 0); break;
							default: return;
							}
						}
						else if (!_stricmp(window->GetName(), "lblAddNote")) ToggleNoteEditMode(1);
						else if (!_stricmp(window->GetName(), "lblEditNote")) ToggleNoteEditMode(2);
						else if (!_stricmp(window->GetName(), "lblDeleteNote")) ToggleNoteEditMode(3);
						else if (!_stricmp(window->GetName(), "lblZoomIn")) SetMapZoom(DWORD(mapZoomIndex - 1), 0, 0, 0);
						else if (!_stricmp(window->GetName(), "lblZoomOut")) SetMapZoom(DWORD(mapZoomIndex + 1), 0, 0, 0);
					}
				}
			}
			break;
		case 4:
			{
				const LONG holodisc = DownloadHolodiscIndex(window->GetName(), downloadsTop);
				if (holodisc >= 0 && value1A4 != holodisc)
				{
					value1A4 = holodisc;
					value1A8 = -1;
				}
				else
				{
					const LONG video = DownloadVideoIndex(window->GetName(), downloadsTop, value1B4);
					if (video < 0) return;
					value1A8 = video;
				}
				RefreshDownloads();
				RefreshDownloadDescription();
				RefreshButtons();
			}
			break;
		}
	}

	LONG PipBoyInterface::SelectedAlarmIndex() const //44AAC0; original Rest prototype only queries.
	{
		return AlarmNameIndex(alarmText);
	}

	void PipBoyInterface::ToggleNoteEditMode(LONG mode) //452490.
	{
		SetNoteEditMode(valueBC == mode ? 0 : DWORD(mode));
	}

	D3DXVECTOR3 PipBoyInterface::MapToWorld(AreaMapWindow* window, float x, float y) //44A4D0.
	{
		float worldX = 0, worldZ = 0;
		JE::AreaMap* map = window->areaMap;
		if (map)
		{
			const float minX = map->minX, minZ = map->minZ;
			const LONG width = window->pictureWidth, height = window->pictureHeight;
			worldX = x / double(width) * (map->maxX - minX) + minX;
			worldZ = (height - y) / height * (map->maxZ - minZ) + minZ;
		}
		return D3DXVECTOR3(worldX, 0, worldZ);
	}

	void PipBoyInterface::HandleMapInput(AreaMapWindow* window, LONG x, LONG y) //452EB0.
	{
		LONG windowX, windowY;
		window->GetAbsolutePosition(windowX, windowY);
		switch (valueBC)
		{
		case 1:
			{
				JE::AreaMap* map = SelectedAreaMap();
				if (!map) return;
				JE::AreaMapNote* target = map->FindNote(noteIndex);
				LONG left, top, width, height;
				if (!target || !window->ComputeNoteRectangle(target, left, top, width, height)) return;
				float scaleX, scaleY;
				::GUI::g_Manager.GetScale(scaleX, scaleY);
				addNote->SetPopupPosition(LONG(static_cast<LONGLONG>((left + width) / scaleX)),
					LONG(static_cast<LONGLONG>((top + height) / scaleY)));
				addNote->Open(this, map, target, 0);
				::GUI::g_Manager.PushModal(addNote);
				return;
			}
		case 2:
			{
				float scaleX, scaleY;
				::GUI::g_Manager.GetScale(scaleX, scaleY);
				AreaMapIcon* icon = window->HitNoteIcon(LONG(static_cast<LONGLONG>(x * scaleX)),
					LONG(static_cast<LONGLONG>(y * scaleY)), &noteIndex);
				JE::AreaMapNote* target = icon ? icon->GetNote() : NULL;
				JE::AreaMap* map = SelectedAreaMap();
				if (!map || !target || target->flag5C) return;
				note = *target;
				LONG left, top, width, height;
				icon->GetRect(left, top, width, height);
				addNote->SetPopupPosition(LONG(static_cast<LONGLONG>((left + width) / scaleX)),
					LONG(static_cast<LONGLONG>((top + height) / scaleY)));
				addNote->Open(this, map, target, 0);
				::GUI::g_Manager.PushModal(addNote);
				return;
			}
		case 3:
			{
				float scaleX, scaleY;
				::GUI::g_Manager.GetScale(scaleX, scaleY);
				const LONG scaledX = LONG(static_cast<LONGLONG>(x * scaleX));
				const LONG scaledY = LONG(static_cast<LONGLONG>(y * scaleY));
				LONG index = -1;
				JE::AreaMapNote* target = window->HitNote(scaledX, scaledY, &index);
				JE::AreaMap* map = SelectedAreaMap();
				if (map && target && !target->flag5C)
				{
					map->RemoveNote(index);
					SetNoteEditMode(0);
					window->RebuildVisibleSamples();
					window->UpdatePartyHover(scaledX, scaledY);
					window->UpdateNoteHover(scaledX, scaledY);
					UpdateNotePopup(scaledX, scaledY);
				}
			}
			break;
		default:
			{
				JE::IWorldClient* clientWorld = static_cast<JE::IWorldClient*>(JE::g_World);
				JE::Map* map = clientWorld->GetCurrentMap();
				JE::AreaMap* selected = SelectedAreaMap();
				if (!map || !selected || map->areaMap != selected) return;
				float scaleX, scaleY;
				::GUI::g_Manager.GetScale(scaleX, scaleY);
				const LONG scaledX = LONG(static_cast<LONGLONG>(x * scaleX));
				const LONG scaledY = LONG(static_cast<LONGLONG>(y * scaleY));
				JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
				DWORD partyID = 0;
				AreaMapIcon* icon = window->HitPartyIcon(scaledX, scaledY, &partyID);
				selected = SelectedAreaMap();
				const DWORD selectedID = client->field0xE0;
				if (icon && selected && selectedID != partyID)
				{
					client->field0xE0 = partyID;
					return;
				}
				const LONG originX = window->originX, originY = window->originY;
				window->GetAbsolutePosition(windowX, windowY);
				const float zoom = g_PipBoyMapZooms[mapZoomIndex];
				D3DXVECTOR3 world = MapToWorld(window,
					((x - windowX) * scaleX + originX) * zoom,
					((y - windowY) * scaleY + originY) * zoom);
				LONG first, row;
				selected->WorldToCell(world, &first, &row);
				if (!selected->IsExplored(first, row)) return;
				OCTREE* octree = map->GetOctree();
				float bounds[6];
				octree->GetLevelBounds(bounds);
				D3DXVECTOR3 position(world.x, bounds[4], world.z);
				float height;
				if (octree->QueryFloorHeight(&position.x, &height, 1, 0, NULL) == 1) position.y = height;
				client->RequestMoveTo(selectedID, position);
			}
			break;
		}
	}

	void PipBoyInterface::ToggleMapMode() //451650.
	{
		mapMode = !mapMode;
		::GUI::Window* first = FindWindow("winMaps1a");
		::GUI::Window* second = FindWindow("winMaps1b");
		::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow("lblMapMode"));
		first->SetHidden(mapMode);
		second->SetHidden(!mapMode);
		primaryMap->SetHidden(!mapMode);
		label->SetStringReference(mapMode ? 415 : 416);
		RefreshMaps();
		QueryMapBounds();
		ClampMapOrigin(0, 0, 0);
		RefreshButtons();
		ShowLilPip(SelectedAreaMap() == NULL, mapMode);
	}

	void PipBoyInterface::QueryMapBounds() //44A730; original prototype leaves query results local.
	{
		if (!secondaryMap) return;
		const LONG pictureWidth = secondaryMap->pictureWidth, pictureHeight = secondaryMap->pictureHeight;
		LONG width, height;
		secondaryMap->GetSize(width, height);
		RECT bounds;
		if (!SelectedExploredBounds(bounds))
		{
			const LONG centerX = pictureWidth / 2;
			const LONG centerY = pictureHeight / 2;
			bounds.top = centerY;
			bounds.bottom = centerY;
			bounds.left = centerX;
			bounds.right = centerX;
			if (value4)
			{
				JE::Entity* entity = JE::GetEntityPtr(value4);
				if (entity)
				{
					D3DXVECTOR3 position;
					position.x = entity->position.x;
					position.y = entity->position.y;
					position.z = entity->position.z;
					LONG x, y;
					secondaryMap->WorldToIntegerMap(position.x,position.z,x,y);
				}
			}
		}
	}

	void PipBoyInterface::UpdateDraggedNote(LONG x, LONG y) //44AE30.
	{
		if (noteIndex < 0 || valueBC != 1) return;
		JE::AreaMap* map = SelectedAreaMap();
		if (!map) return;
		JE::AreaMapNote* target = map->FindNote(noteIndex);
		if (!target) return;
		float scaleX, scaleY;
		::GUI::g_Manager.GetScale(scaleX, scaleY);
		AreaMapWindow* window;
		if (secondaryMap->Contains(x, y)) window = secondaryMap;
		else if (mapMode && primaryMap->Contains(x, y)) window = primaryMap;
		else
		{
			if (target->flag5D) return;
			target->flag5D = 1;
			map->changes |= 2;
			return;
		}
		const LONG originX = window->originX, originY = window->originY;
		LONG left, top;
		window->GetAbsolutePosition(left, top);
		const float zoom = g_PipBoyMapZooms[mapZoomIndex];
		const D3DXVECTOR3 position = MapToWorld(window,
			((x - left) * scaleX + originX) * zoom, ((y - top) * scaleY + originY) * zoom);
		target->position[0] = position.x;
		target->position[1] = position.y;
		target->position[2] = position.z;
		target->flag5D = 0;
		map->changes |= 2;
	}

	void PipBoyInterface::SetMapZoom(DWORD index, BYTE anchored, LONG x, LONG y) //452560.
	{
		if (index > 4 || index == DWORD(mapZoomIndex)) return;
		const float oldZoom = g_PipBoyMapZooms[mapZoomIndex], newZoom = g_PipBoyMapZooms[index];
		mapZoomIndex = LONG(index);
		const LONG originX = secondaryMap->originX, originY = secondaryMap->originY;
		LONG width, height;
		secondaryMap->GetSize(width, height);
		float scaleX, scaleY;
		::GUI::g_Manager.GetScale(scaleX, scaleY);
		width = LONG(static_cast<LONGLONG>(width * scaleX));
		height = LONG(static_cast<LONGLONG>(height * scaleY));
		if (mapMode)
		{
			LONG firstWidth, firstHeight;
			primaryMap->GetSize(firstWidth, firstHeight);
			height += LONG(static_cast<LONGLONG>(firstHeight * scaleY));
		}
		primaryMap->zoom = newZoom;
		primaryMap->dirty = 1;
		secondaryMap->zoom = newZoom;
		secondaryMap->dirty = 1;
		LONG anchorX, anchorY;
		if (anchored) { anchorX = x; anchorY = y; }
		else
		{
			anchorX = LONG(static_cast<LONGLONG>((width * .5 + originX) * oldZoom));
			anchorY = LONG(static_cast<LONGLONG>((height * .5 + originY) * oldZoom));
			x = anchorX;
			y = anchorY;
		}
		const double inverse = 1.0 / newZoom;
		const double ratio = oldZoom * inverse;
		LONG difference = anchorX - LONG(static_cast<LONGLONG>(x / ratio));
		mapX = LONG(static_cast<LONGLONG>(difference * inverse + mapX));
		difference = anchorY - LONG(static_cast<LONGLONG>(y / ratio));
		mapY = LONG(static_cast<LONGLONG>(difference * inverse + mapY));
		ClampMapOrigin(1, 0, 0);
		RefreshMaps();
		::GUI::MouseState mouse;
		::GUI::g_Manager.GetMouseState(mouse);
		UpdateDraggedNote(mouse.x, mouse.y);
		if (secondaryMap)
		{
			secondaryMap->RebuildVisibleSamples();
			secondaryMap->UpdatePartyHover(mouse.x, mouse.y);
			secondaryMap->UpdateNoteHover(mouse.x, mouse.y);
		}
		if (mapMode && primaryMap)
		{
			primaryMap->RebuildVisibleSamples();
			primaryMap->UpdatePartyHover(mouse.x, mouse.y);
			primaryMap->UpdateNoteHover(mouse.x, mouse.y);
		}
		UpdateNotePopup(mouse.x, mouse.y);
	}
}
