#include "game/WorldMapInterface.hpp"
#include "game/POIPicture.hpp"
#include "game/POIWaypoint.hpp"
#include "entity/WorldCollections.hpp"
#include "entity/World.hpp"
#include "gui/Label.hpp"
#include "gui/LineSystem.hpp"
#include "gui/ScrollBar.hpp"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>

namespace F3
{
	void WorldMapInterface::HidePictures()
	{
		for (std::list<POIPicture*>::iterator i = pictures.begin(); i != pictures.end(); ++i)
		{
			POIPicture* picture = *i;
			if (picture)
			{
				picture->SetHidden(1);
				picture->label->SetHidden(1);
			}
		}
	}

	POIPicture* WorldMapInterface::FindVisibleLocation(DWORD id)
	{
		for (std::list<POIPicture*>::iterator i = pictures.begin(); i != pictures.end(); ++i)
		{
			POIPicture* picture = *i;
			if (picture && !picture->GetHidden() && picture->location->values[3] == id)
				return picture;
		}
		return NULL;
	}

	void WorldMapInterface::HideWaypoints()
	{
		for (std::list<POIWaypoint*>::iterator i = waypoints.begin(); i != waypoints.end(); ++i)
		{
			POIWaypoint* waypoint = *i;
			if (waypoint)
			{
				waypoint->SetHidden(1);
				waypoint->lineSystem->Clear();
			}
		}
	}

	void WorldMapInterface::ClearRoutes()
	{
		for (std::list<WorldMapRouteRecord*>::iterator i = routes.begin();
			i != routes.end(); ++i)
		{
			WorldMapRouteRecord* route = *i;
			if (route)
			{
				route->ReleaseChild(); //Required original458770, no substitute owner.
				::operator delete(route); //Release the captured allocation before advancing its node.
			}
		}
		routes.clear(); //Only now erase nodes; preserve the list/sentinel and other state.
	}

	POIPicture* WorldMapInterface::AcquirePicture()
	{
		for (std::list<POIPicture*>::iterator i = pictures.begin(); i != pictures.end(); ++i)
		{
			POIPicture* picture = *i;
			if (picture && picture->GetHidden())
				return picture;
		}
		POIPicture* picture = new POIPicture;
		AddWindow(picture); //Publish the actual root before configuring its children.
		picture->SetName(std::string("POIIcon"));
		picture->ApplySkin(pictureSkin);
		picture->Resize(pictureWidth, pictureHeight);
		picture->SetViewport(viewport);
		picture->SetHidden(1);
		::GUI::Label* label = new ::GUI::Label(3);
		picture->AddChild(label);
		label->ApplySkin(labelSkin);
		label->SetFont(fontName, 0, fontSize, 1);
		label->Resize(labelWidth, labelHeight);
		label->SetAlignment(2);
		label->SetTextColor(textColor);
		label->SetViewport(viewport);
		label->SetValue3B(1);
		label->SetHidden(1);
		picture->label = label;
		pictures.push_back(picture);
		return picture;
	}

	POIWaypoint* WorldMapInterface::AcquireWaypoint()
	{
		for (std::list<POIWaypoint*>::iterator i = waypoints.begin(); i != waypoints.end(); ++i)
		{
			POIWaypoint* waypoint = *i;
			if (waypoint && waypoint->GetHidden())
				return waypoint;
		}
		POIWaypoint* waypoint = new POIWaypoint;
		AddWindow(waypoint);
		waypoint->SetName(std::string("POIWaypoint"));
		waypoint->ApplySkin(std::string("quickslot.ini"));
		waypoint->SetFont(std::string("arial"), 0, 20, 1);
		waypoint->SetText(std::string("x"));
		waypoint->Resize(32, 32);
		waypoint->SetAlignment(2);
		waypoint->SetTextColor(0xFF000000);
		waypoint->SetViewport(viewport);
		waypoint->SetValue3B(1);
		waypoint->SetHidden(1);
		waypoints.push_back(waypoint);
		return waypoint;
	}

	void WorldMapInterface::ResetCurrentMarker()
	{
		POIPicture* picture = FindVisibleLocation(currentLocationID);
		if (picture)
		{
			const LONG x = picture->coordinateX;
			const LONG y = picture->coordinateY;
			POIWaypoint* waypoint = AcquireWaypoint();
			waypoint->coordinateX = LONG(DWORD(x) + 20);
			waypoint->coordinateY = LONG(DWORD(y) + 16);
			waypoint->valueE6C = 0.0f;
			waypoint->valueE70 = 0.0f;
			waypoint->valueE74 = 0.0f;
			waypoint->valueE78 = 0.0f;
			waypoint->SetHidden(0);
			waypoint->BringToFront();
		}
	}

	void WorldMapInterface::ClearTiles()
	{
		for (LONG row = 0; row < tileRows; ++row)
		{
			for (LONG column = 0; column < tileColumns; ++column)
			{
				::GUI::Picture* picture = tiles[DWORD(column) + DWORD(row) * DWORD(tileColumns)];
				if (picture)
					delete picture;
			}
		}
		tileRows = 0;
		tileColumns = 0;
		tilePrefix.assign("");
		std::vector< ::GUI::Picture*>().swap(tiles);
	}

	void WorldMapInterface::LoadTiles(const std::string& name, BYTE force)
	{
		if (!map)
			return;
		const std::string prefix = name.substr(0, name.rfind('.'));
		if (!_stricmp(prefix.c_str(), tilePrefix.c_str()) && !force)
			return;
		ClearTiles();
		tilePrefix = prefix;
		tileRows = LONG(map->value28);
		tileColumns = LONG(map->value2C);
		tiles.resize(DWORD(tileRows) * DWORD(tileColumns), NULL);
		for (LONG row = 0; row < tileRows; ++row)
		{
			const LONG y = LONG(DWORD(row) << 8);
			for (LONG column = 0; column < tileColumns; ++column)
			{
				char filename[64];
				std::sprintf(filename, "%s%02d%02d.bmp", tilePrefix.c_str(), column, row);
				::GUI::Picture* picture = new ::GUI::Picture(2);
				tiles[DWORD(column) + DWORD(row) * DWORD(tileColumns)] = picture;
				//Each callback is followed by the original live vector/count reload.
				tiles[DWORD(column) + DWORD(row) * DWORD(tileColumns)]->ApplySkin("quickslot.ini");
				tiles[DWORD(column) + DWORD(row) * DWORD(tileColumns)]->SetFrameVisible(0);
				tiles[DWORD(column) + DWORD(row) * DWORD(tileColumns)]->SetValue3B(1);
				tiles[DWORD(column) + DWORD(row) * DWORD(tileColumns)]->Resize(256, 256);
				tiles[DWORD(column) + DWORD(row) * DWORD(tileColumns)]->LoadTexture(
					std::string(filename), 1, 0.0f, 0.0f, -1.0f, -1.0f, 1);
				tiles[DWORD(column) + DWORD(row) * DWORD(tileColumns)]->SetViewport(viewport);
				tiles[DWORD(column) + DWORD(row) * DWORD(tileColumns)]->SetAbsolutePosition(
					LONG(DWORD(column) << 8), y);
				AddWindow(tiles[DWORD(column) + DWORD(row) * DWORD(tileColumns)]);
			}
		}
		vertical->SetPageSize(pageHeight);
		vertical->SetMaximum(LONG(DWORD(tileRows) << 8));
		vertical->SetStep(10);
		vertical->SetPageStep(100);
		vertical->SetValue(0, 1);
		horizontal->SetPageSize(pageWidth);
		horizontal->SetMaximum(LONG(DWORD(tileColumns) << 8));
		horizontal->SetStep(10);
		horizontal->SetPageStep(100);
		horizontal->SetValue(0, 1);
	}

	LONG WorldMapInterface::ConvertX(LONG x)
	{
		::GUI::ScrollBar* scrollbar = horizontal;
		const LONG value = scrollbar->GetValue();
		return LONG(DWORD(x) + DWORD(originX) - DWORD(value));
	}

	LONG WorldMapInterface::ConvertY(LONG y)
	{
		::GUI::ScrollBar* scrollbar = vertical;
		const LONG value = scrollbar->GetValue();
		return LONG(DWORD(y) + DWORD(originY) - DWORD(value));
	}

	void WorldMapInterface::Layout()
	{
		for (LONG row = 0; row < tileRows; ++row)
		{
			const LONG y = LONG(DWORD(row) << 8);
			for (LONG column = 0; column < tileColumns; ++column)
			{
				::GUI::Picture* picture = tiles[DWORD(column) + DWORD(row) * DWORD(tileColumns)];
				const LONG screenY = ConvertY(y);
				const LONG screenX = ConvertX(LONG(DWORD(column) << 8));
				picture->SetAbsolutePosition(screenX, screenY);
			}
		}
		for (std::list<POIPicture*>::iterator i = pictures.begin(); i != pictures.end(); ++i)
		{
			POIPicture* picture = *i;
			if (picture && !picture->GetHidden())
			{
				const LONG x = picture->coordinateX;
				const LONG y = picture->coordinateY;
				const LONG screenY = ConvertY(y);
				const LONG screenX = ConvertX(x);
				picture->SetAbsolutePosition(screenX, screenY);
			}
		}
		std::list<POIWaypoint*>::iterator previous;
		float distance = 0.0f;
		for (std::list<POIWaypoint*>::iterator i = waypoints.begin(); i != waypoints.end(); ++i)
		{
			POIWaypoint* waypoint = *i;
			if (!waypoint || waypoint->GetHidden())
				break;
			const LONG x = waypoint->coordinateX;
			const LONG y = waypoint->coordinateY;
			const LONG screenY = LONG(DWORD(ConvertY(y)) - 18);
			const LONG screenX = LONG(DWORD(ConvertX(x)) - 16);
			waypoint->SetAbsolutePosition(screenX, screenY);
			waypoint->lineSystem->Clear();
			//The original compares with the live head after the preceding callbacks.
			if (i != waypoints.begin())
			{
				POIWaypoint* prior = *previous;
				if (prior)
				{
					const LONG priorX = prior->coordinateX;
					const LONG priorY = prior->coordinateY;
					const LONG priorScreenY = ConvertY(priorY);
					const LONG priorScreenX = ConvertX(priorX);
					const LONG currentScreenY = ConvertY(y);
					const LONG currentScreenX = ConvertX(x);
					::GUI::LineSystem* lines = waypoint->lineSystem;
					const BYTE red = lines->red;
					const BYTE green = lines->green;
					const BYTE blue = lines->blue;
					const BYTE alpha = lines->alpha;
					lines->AddLine(currentScreenX, currentScreenY, priorScreenX, priorScreenY,
						red, green, blue, alpha, red, green, blue, alpha);
					waypoint->valueE6C = float(LONG(DWORD(x) - DWORD(priorX)));
					waypoint->valueE70 = float(LONG(DWORD(y) - DWORD(priorY)));
					const double squared = double(waypoint->valueE6C) * waypoint->valueE6C
						+ double(waypoint->valueE70) * waypoint->valueE70
						+ double(waypoint->valueE74) * waypoint->valueE74;
					distance = float(std::sqrt(squared) + double(distance));
					waypoint->valueE78 = distance;
				}
			}
			previous = i;
		}
	}

	void WorldMapInterface::Refresh()
	{
		if (!map)
			return;
		LoadTiles(map->value30, 0);
		HidePictures();
		HideWaypoints();
		ClearRoutes(); //Required original call; its concrete route lifetime remains undefined.
		std::list<JE::WorldMapLocation*>& source = map->locations;
		valueE34 = DWORD(-1);
		std::list<JE::WorldMapLocation*> locations(source);
		for (std::list<JE::WorldMapLocation*>::iterator i = locations.begin();
			i != locations.end(); ++i)
		{
			JE::WorldMapLocation* location = *i;
			if (location && (location->values[0] & 1))
			{
				POIPicture* picture = AcquirePicture();
				picture->location = location;
				LONG x, y;
				location->GetCoordinates(x, y);
				picture->coordinateX = x;
				picture->coordinateY = y;
				picture->SetAbsolutePosition(x, y);
				std::string texture;
				location->GetTextureName(texture);
				picture->LoadTexture(texture, 0, 0.0f, 0.0f, -1.0f, -1.0f, 1);
				picture->SetHidden(0);
				picture->BringToFront();
				picture->label->SetAbsolutePosition(LONG(DWORD(x) + DWORD(labelOffsetX)),
					LONG(DWORD(y) + DWORD(labelOffsetY)));
				picture->label->SetStringReference(location->GetLabelReference());
				picture->label->SetHidden(0);
			}
		}
		ResetCurrentMarker();
		Layout();
	}

	void WorldMapInterface::Configure(const char* name, DWORD location)
	{
		JE::IWorld* world = JE::g_World;
		map = world->GetWorldMap(name);
		currentLocationID = location;
		Refresh();
	}
}
