#include "WorldCollections.hpp"
#include "Store.hpp"
#include "resources/ResourceSystem.hpp"
#include <algorithm>

namespace JE
{
	Party::Party() {}
	Party::~Party() {} // The real vector supplies deallocation; no virtual Cleanup call.
	BYTE Party::Add(Entity* entity)
	{
		if (!CanAdd()) return 0;
		members.push_back(entity);
		return 1;
	}
	BYTE Party::CanAdd() { return 1; }
	void Party::Cleanup() { while (!members.empty()) Remove(members.front()); }
	BYTE Party::Contains(Entity* entity)
	{
		for (std::vector<Entity*>::iterator i = members.begin(); i != members.end(); ++i)
			if (*i == entity) return 1;
		return 0;
	}
	BYTE Party::Remove(Entity* entity)
	{
		BYTE result = 0;
		for (std::vector<Entity*>::iterator i = members.begin(); i != members.end(); ++i)
			if (*i == entity)
			{
				members.erase(i);
				result = 1;
				break;
			}
		return result;
	}
	JournalEntry::JournalEntry() throw() { value4 = value8 = DWORD(-1); }
	JournalEntry::~JournalEntry() {}
	TaskStatus::TaskStatus() throw() : value4(DWORD(-1)), value8(DWORD(-1)), valueC(0) {}
	TaskStatus::~TaskStatus() {}
}

	Journal::Journal() { entries.clear(); }
	Journal::~Journal()
	{
		entries.clear(); // Original base never deletes JournalEntry payloads.
	}
	void Journal::Cleanup() { entries.clear(); }
namespace JE
{
	EncyclopediaEntry::EncyclopediaEntry()
	{
		id = 0;
		value4 = 0;
		flag8 = 0;
		valueC = "";
		values28.clear();
		values34.clear();
		value40 = 0;
		icon44 = "";
	}
	EncyclopediaEntry::~EncyclopediaEntry() {} //58F330; real strings/lists supply member destruction.
}

	Encyclopedia::Encyclopedia() { entries.clear(); }
	Encyclopedia::~Encyclopedia()
	{
		Encyclopedia::Cleanup();
	}
	void Encyclopedia::Initialize() { Cleanup(); }
	JE::EncyclopediaEntry* Encyclopedia::FindEntry(DWORD id) //58F2B0.
	{
		for (std::list<JE::EncyclopediaEntry*>::iterator i = entries.begin(); i != entries.end(); ++i)
		{
			JE::EncyclopediaEntry* entry = *i;
			if (entry && entry->id == id) return entry;
		}
		return NULL;
	}
	void Encyclopedia::AddEntry(DWORD id, DWORD value40, const char* icon, DWORD value4) //58F5B0.
	{
		JE::EncyclopediaEntry* entry = new JE::EncyclopediaEntry;
		entry->id = id;
		entry->value40 = value40;
		if (icon && JE::Resources::g_pResourceSystem->Contains(icon))
		{
			entry->icon44 = std::string(icon);
		}
		else
		{
			entry->icon44 = std::string("icon_unknown.bmp");
		}
		entry->value4 = value4;
		entries.push_back(entry);
	}
	void Encyclopedia::Cleanup()
	{
		for (std::list<JE::EncyclopediaEntry*>::iterator i = entries.begin(); i != entries.end(); ++i)
			if (*i) delete *i;
		entries.clear();
	}
namespace JE
{
	StoreManager::StoreManager() { stores.clear(); }
	StoreManager::~StoreManager()
	{
		StoreManager::Cleanup();
	}
	void StoreManager::Cleanup() { ClearStores(); } //5A21A0, native virtual wrapper.
	CStore* StoreManager::GetStore(const char* name) //5A23B0.
	{
		for (std::list<CStore*>::iterator i = stores.begin(); i != stores.end(); ++i)
		{
			CStore* store = *i; //5A23E0 captures the payload; a match returns that same pointer.
			if (store && !_stricmp(store->name.c_str(), name)) return store;
		}
		if (Resources::g_pResourceSystem->Contains(name))
		{
			CStore* store = new CStore;
			store->Read(name);
			stores.push_back(store);
		}
		return NULL; //Original returns NULL even after successfully appending a new store.
	}
	namespace Common
	{
		Polygon::Polygon() {}
		Polygon::~Polygon() {}
		void Polygon::ReleasePoints() { points.clear(); } //VC7 clear releases the buffer.
		Polygon& Polygon::operator=(const Polygon& source)
		{
			points.clear(); //Original self-assignment empties the polygon.
			for (std::vector<D3DXVECTOR3>::const_iterator point=source.points.begin();
				point!=source.points.end(); ++point)
			{
				D3DXVECTOR3 value;
				value.x=point->x;
				value.y=point->y;
				value.z=point->z;
				points.push_back(value);
			}
			return *this;
		}
		void Polygon::Translate(const D3DXVECTOR3& translation)
		{
			for (std::vector<D3DXVECTOR3>::iterator point=points.begin();point!=points.end();++point)
			{
				point->x=float(double(point->x)+translation.x);
				point->y=float(double(point->y)+translation.y);
				point->z=float(double(point->z)+translation.z);
			}
		}
	}
	WorldMapLocation::WorldMapLocation()
	{
		values[0] = 0;
		values[1] = 0;
		values[2] = 0;
		values[3] = 0;
		values[4] = 0;
		values[5] = 0;
		values[6] = 0;
		values[7] = 0;
		value20 = "";
		value3C = "";
		value58 = "";
	}
	WorldMapLocation::~WorldMapLocation() {} //599DF0.
	void WorldMapLocation::GetCoordinates(LONG& x, LONG& y) const
	{
		x = LONG(values[1]);
		y = LONG(values[2]);
	}
	DWORD WorldMapLocation::GetLabelReference() const
	{
		const BYTE flag = BYTE(values[0] >> 2);
		return flag & 1 ? values[4] : values[6];
	}
	std::string& WorldMapLocation::GetTextureName(std::string& output) const
	{
		return output.assign(value20, 0, std::string::npos);
	}
	std::string& WorldMapLocation::GetOnEnterScript(std::string& output) const //59A1C0.
	{
		return output.assign(value58, 0, std::string::npos);
	}
	WorldMapRegion::WorldMapRegion()
	{
		value0 = 0;
		value4 = 0;
		value8 = 0;
		valueC = "";
		polygon28.ReleasePoints();
	}
	WorldMapRegion::~WorldMapRegion() { polygon28.ReleasePoints(); }
	void WorldMapRegion::AddPoint(LONG x, LONG y)
	{
		polygon28.points.push_back(D3DXVECTOR3(static_cast<float>(x), 0, static_cast<float>(y)));
	}
	WorldMap::WorldMap()
	{
		resourceName = "";
		value20 = 0;
		flag24 = 1;
		value28 = 0;
		value2C = 0;
		value30 = "";
		value4C = "";
		locations.clear();
		regions.clear();
	}
	WorldMap::~WorldMap()
	{
		ClearLocations();
		ClearRegions();
	}
	void WorldMap::ClearLocations()
	{
		for (std::list<WorldMapLocation*>::iterator i = locations.begin(); i != locations.end(); ++i)
			if (*i) delete *i;
		locations.clear();
	}
	void WorldMap::ClearRegions()
	{
		for (std::list<WorldMapRegion*>::iterator i = regions.begin(); i != regions.end(); ++i)
			if (*i) delete *i;
		regions.clear();
	}
	void WorldMap::Cleanup()
	{
		ClearLocations();
		ClearRegions();
	}
}
