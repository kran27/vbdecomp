#include "game/PipBoyInterface.hpp"
#include "game/AreaMapWindow.hpp"
#include "gui/Manager.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include <climits>

namespace
{
	LONG MapTruncate(double value) { return LONG(static_cast<LONGLONG>(value)); }
}

namespace F3
{
	void PipBoyInterface::CenterMap() //44D710.
	{
		AreaMapWindow* map = secondaryMap;
		if (!map || !SelectedAreaMap()) return;
		const LONG sourceWidth = map->pictureWidth, sourceHeight = map->pictureHeight;
		if (sourceWidth <= 0 || sourceHeight <= 0) return;
		const float zoom = g_PipBoyMapZooms[mapZoomIndex];
		RECT bounds;
		if (!SelectedExploredBounds(bounds))
		{
			const LONG centerX = sourceWidth / 2, centerY = sourceHeight / 2;
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
					LONG x,y;
					secondaryMap->WorldToIntegerMap(position.x,position.z,x,y);
					bounds.left = x;
					bounds.top = y;
					bounds.right = x;
					bounds.bottom = y;
				}
			}
		}
		const float inverse = 1.0f / zoom;
		bounds.left = MapTruncate(bounds.left * inverse); bounds.top = MapTruncate(bounds.top * inverse);
		bounds.right = MapTruncate(bounds.right * inverse); bounds.bottom = MapTruncate(bounds.bottom * inverse);
		LONG width,height;
		secondaryMap->GetSize(width,height);
		float scaleX,scaleY; ::GUI::g_Manager.GetScale(scaleX,scaleY);
		width = MapTruncate(width * scaleX); height = MapTruncate(height * scaleY);
		if (mapMode)
		{
			LONG firstWidth,firstHeight; primaryMap->GetSize(firstWidth,firstHeight);
			height += MapTruncate(firstHeight * scaleY);
		}
		mapX = bounds.left + (bounds.right - width - bounds.left) / 2;
		mapY = bounds.top + (bounds.bottom - height - bounds.top) / 2;
		ClampMapOrigin(1,0,0);
	}

	void PipBoyInterface::ClampMapOrigin(BYTE clamp, LONG dx, LONG dy) //44D460.
	{
		maximumX = LONG_MIN;
		maximumY = LONG_MIN;
		AreaMapWindow* map = secondaryMap;
		minimumX = LONG_MAX;
		minimumY = LONG_MAX;
		if (!map || !SelectedAreaMap()) return;
		RECT bounds;
		if (!SelectedExploredBounds(bounds))
		{
			map = secondaryMap;
			const LONG centerX = map->pictureWidth / 2, centerY = map->pictureHeight / 2;
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
					LONG x,y;
					secondaryMap->WorldToIntegerMap(position.x,position.z,x,y);
					bounds.left = x;
					bounds.top = y;
					bounds.right = x;
					bounds.bottom = y;
				}
			}
		}
		LONG width,height; secondaryMap->GetSize(width,height);
		float scaleX,scaleY; ::GUI::g_Manager.GetScale(scaleX,scaleY);
		width = MapTruncate(width * scaleX); height = MapTruncate(height * scaleY);
		const LONG secondHeight = height;
		if (mapMode)
		{
			LONG firstWidth,firstHeight; primaryMap->GetSize(firstWidth,firstHeight);
			height += MapTruncate(firstHeight * scaleY);
		}
		const float inverse = 1.0f / g_PipBoyMapZooms[mapZoomIndex];
		const LONG minX = MapTruncate(8.0f - (width - bounds.left * inverse));
		minimumX = minX;
		const LONG maxX = MapTruncate(bounds.right * inverse - 8.0f);
		maximumX = maxX;
		const LONG minY = MapTruncate(8.0f - (height - bounds.top * inverse));
		minimumY = minY;
		const LONG maxY = MapTruncate(bounds.bottom * inverse - 8.0f);
		maximumY = maxY;
		if (clamp)
		{
			if (mapX < minX && ((!dx && !dy) || dx < 0)) mapX = minX;
			if (mapX > maxX && ((!dx && !dy) || dx > 0)) mapX = maxX;
			if (mapY < minY && ((!dx && !dy) || dy < 0)) mapY = minY;
			if (mapY > maxY && ((!dx && !dy) || dy > 0)) mapY = maxY;
		}
		secondaryMap->SetOrigin(mapX,mapY);
		if (mapMode) primaryMap->SetOrigin(mapX,mapY + secondHeight);
		RefreshButtons();
	}
}
