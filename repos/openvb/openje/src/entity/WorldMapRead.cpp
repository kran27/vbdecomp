#include "WorldCollections.hpp"
#include "resources/ResourceSystem.hpp"
#include <cstring>

namespace JE
{
	WorldMapLocation* WorldMap::FindLocation(DWORD id) //599DB0.
	{
		for (std::list<WorldMapLocation*>::iterator i=locations.begin();i!=locations.end();++i)
		{
			WorldMapLocation* location = *i;
			if (location && location->values[3]==id) return location;
		}
		return NULL;
	}

	DWORD WorldMap::ReadWorldMapDword(const BYTE*& cursor) //Actual5A1E50, also used by ENC.
	{
		DWORD value;
		std::memcpy(&value,cursor,sizeof(value));
		cursor+=sizeof(value);
		return value;
	}
	const char* WorldMap::ReadWorldMapString(const BYTE*& cursor) //5A1E60, includes terminal NUL.
	{
		const char* value=reinterpret_cast<const char*>(cursor);
		while (*cursor) ++cursor;
		++cursor;
		return value;
	}
	void WorldMap::Read(const char* name) //59A440, appends without clearing existing records.
	{
		Resources::ResourceData* resource=Resources::g_pResourceSystem->Demand(name,1);
		if (!resource) return;
		resourceName=std::string(name);
		const BYTE* cursor=static_cast<const BYTE*>(resource->data);
		value20=ReadWorldMapDword(cursor);
		value30=std::string(ReadWorldMapString(cursor));
		value4C=std::string(ReadWorldMapString(cursor));
		flag24=ReadWorldMapDword(cursor)==0;
		value2C=ReadWorldMapDword(cursor);
		value28=ReadWorldMapDword(cursor);
		LONG remaining=static_cast<LONG>(ReadWorldMapDword(cursor));
		while (remaining>0)
		{
			WorldMapLocation* location=new WorldMapLocation;
			location->values[6]=ReadWorldMapDword(cursor);
			location->values[4]=ReadWorldMapDword(cursor);
			location->values[7]=ReadWorldMapDword(cursor);
			location->values[5]=ReadWorldMapDword(cursor);
			location->value20=std::string(ReadWorldMapString(cursor));
			location->value58=std::string(ReadWorldMapString(cursor));
			//Original+3C remains constructor-empty; the file supplies two strings.
			location->values[1]=ReadWorldMapDword(cursor);
			location->values[2]=ReadWorldMapDword(cursor);
			location->values[0]=ReadWorldMapDword(cursor);
			location->values[3]=ReadWorldMapDword(cursor);
			locations.push_back(location);
			--remaining;
		}
		remaining=static_cast<LONG>(ReadWorldMapDword(cursor));
		while (remaining>0)
		{
			WorldMapRegion* region=new WorldMapRegion;
			region->value0=ReadWorldMapDword(cursor);
			region->value4=ReadWorldMapDword(cursor);
			region->value8=ReadWorldMapDword(cursor);
			region->valueC=std::string(ReadWorldMapString(cursor));
			LONG points=static_cast<LONG>(ReadWorldMapDword(cursor));
			while (points>0)
			{
				const LONG x=static_cast<LONG>(ReadWorldMapDword(cursor));
				const LONG y=static_cast<LONG>(ReadWorldMapDword(cursor));
				region->AddPoint(x,y);
				--points;
			}
			std::list<WorldMapRegion*>::iterator position=regions.begin();
			for (;position!=regions.end();++position)
				if (*position && static_cast<LONG>(region->value8)>=static_cast<LONG>((*position)->value8))
				{
					regions.insert(position,region);
					break;
				}
			if (position==regions.end()) regions.push_back(region); //59A9AE signed descending, new before equal.
			--remaining;
		}
		//59AA2A releases on normal return only. No new bounds/version checks,
		//transaction, failure guard or cleanup of unpublished partial records.
		Resources::g_pResourceSystem->ReleaseResource(resource);
	}
}
