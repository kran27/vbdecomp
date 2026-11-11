#pragma once

#include "gui/Interface.hpp"
#include <list>
#include <string>
#include <vector>

class Gfx_Viewport;
class POIPicture;
class POIWaypoint;
namespace JE { class WorldMap; }
namespace GUI { class Picture; class ScrollBar; }

namespace F3
{

	//Partial type for the observed route-list payload. No storage/layout or
	//producer is established. Undefined lifetime prevents allocating this type.
	class WorldMapRouteRecord
	{
		friend class WorldMapInterface;
		void ReleaseChild(); //458770, declared-only until the actual child is known.
		WorldMapRouteRecord();
		~WorldMapRouteRecord();
		WorldMapRouteRecord(const WorldMapRouteRecord&);
		WorldMapRouteRecord& operator=(const WorldMapRouteRecord&);
	};

	// Descriptive source name for the original unproduced WorldMap screen.
	// Only observed fields are represented; original offsets are annotations,
	// not native layout claims. Construction, destruction and publication are
	// deliberately unavailable until their original owners are recovered.
	class WorldMapInterface : public ::GUI::Interface
	{
	public:
		void HidePictures(); //458AC0.
		POIPicture* FindVisibleLocation(DWORD id); //458B10.
		void HideWaypoints(); //458B70.
		POIPicture* AcquirePicture(); //459150, no stack arguments.
		POIWaypoint* AcquireWaypoint(); //4593A0.
		void ResetCurrentMarker(); //459680.
		void ClearTiles(); //458FE0.
		void LoadTiles(const std::string& name, BYTE force); //4596F0, RET8.
		LONG ConvertX(LONG x); //458730.
		LONG ConvertY(LONG y); //458750.
		void Layout(); //458BC0.
		void Refresh(); //459C50; route-child ownership remains unresolved.
		void Configure(const char* name, DWORD location); //459E70, RET8.

	private:
		WorldMapInterface(); //No recovered constructor/signature or producer.
		virtual ~WorldMapInterface(); //No recovered screen destructor.
		WorldMapInterface(const WorldMapInterface&);
		WorldMapInterface& operator=(const WorldMapInterface&);
		void ClearRoutes(); //458E70, actual traversal;458770 remains undefined.

		// Unknown and unused intervening storage is omitted, never padded.
		JE::WorldMap* map; //Original34, borrowed; its producer is unrecovered.
		LONG pictureWidth, pictureHeight; //Original D68/D6C.
		std::string pictureSkin; //D70.
		LONG labelWidth, labelHeight; //D8C/D90.
		std::string labelSkin, fontName; //D94/DB0.
		LONG fontSize; //DCC.
		LONG labelOffsetX, labelOffsetY; //DD0/DD4.
		DWORD textColor; //DD8.
		std::vector< ::GUI::Picture*> tiles; //DDC, owns the published tile windows.
		std::string tilePrefix; //DEC.
		LONG tileRows, tileColumns; //E08/E0C, signed loop bounds.
		std::list<POIPicture*> pictures; //E10; borrowed published root windows.
		std::list<POIWaypoint*> waypoints; //E1C; borrowed published root windows.
		DWORD currentLocationID; //E28.
		DWORD valueE34; //Refresh stores FFFFFFFF; further role unproved.
		std::list<WorldMapRouteRecord*> routes; //E4C; payload release/free, then node clear.
		::GUI::ScrollBar* vertical; //E58, borrowed.
		::GUI::ScrollBar* horizontal; //E5C, borrowed.
		Gfx_Viewport* viewport; //E60, borrowed.
		LONG originX, originY; //E68/E6C.
		LONG pageWidth, pageHeight; //E70/E74.
	};
}
