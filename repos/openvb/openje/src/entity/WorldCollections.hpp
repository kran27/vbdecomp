#pragma once

#include <windows.h>
#include <d3dx8math.h>
#include <list>
#include <string>
#include <vector>

namespace JE
{
	class Entity;
	class Party
	{
	public:
		Party(); // 0x5A0760.
		~Party(); // 0x5A06E0, NONvirtual; vector storage only.
		virtual BYTE Add(Entity* entity); // 0x5A0780; duplicates permitted.
		virtual BYTE CanAdd(); // Shared408D30, actual return1.
		virtual void Cleanup(); // 0x5A0680, repeatedly calls virtual Remove(front).
		virtual BYTE Contains(Entity* entity); // 0x5A06B0.
		virtual BYTE Remove(Entity* entity); // 0x5A0710, first occurrence only.
		std::vector<Entity*> members; // Original+4; pointer use confirmed46C980.
	};
	class JournalEntry
	{
	public:
		JournalEntry() throw(); //0x5A04D0, actual nonthrowing scalar leaf.
		virtual ~JournalEntry(); // 0x5A04F0.
		DWORD value4, value8; // Both initialFFFFFFFF.
	};
	class TaskEntry : public JournalEntry
	{
	public:
		TaskEntry(); // 0x46C2B0.
		virtual ~TaskEntry(); // 0x46C2D0.
	};
	class TaskStatus
	{
	public:
		TaskStatus() throw(); //46C2E0, actual nonthrowing scalar leaf.
		virtual ~TaskStatus(); // 0x46C300.
		DWORD value4, value8, valueC; // InitialFFFFFFFF,FFFFFFFF,0.
	};
}

struct Journal
	{
	public:
		Journal(); // 0x5A0520.
		virtual ~Journal(); // 0x5A0560; list nodes only.
		virtual void Cleanup(); // 0x5A05C0; list nodes only, unlike GameJournal.
		void AddEntry(DWORD entry,DWORD value); //5A0610, direct base insertion without duplicate check.
		std::list<JE::JournalEntry*> entries; // Original+4, base does not delete payloads.
	};
namespace JE
{
	typedef ::Journal Journal;
	struct EncyclopediaEntry
	{
		EncyclopediaEntry(); //58F4A0.
		~EncyclopediaEntry(); //58F330, ordinary reverse member lifetime; nonvirtual.
		DWORD id, value4;
		BYTE flag8;
		std::string valueC;
		std::list<DWORD> values28, values34;
		DWORD value40;
		std::string icon44;
	};
}

struct Encyclopedia
	{
	public:
		Encyclopedia(); // 0x58F2F0.
		~Encyclopedia(); // 0x58F450, NONvirtual, actual owning Cleanup first.
		virtual void Initialize(); // 0x58F2A0, virtual Cleanup only.
		virtual void AddEntry(DWORD id, DWORD value40, const char* icon, DWORD value4); // 0x58F5B0.
		virtual void Cleanup(); // 0x58F3E0, deletes actual records before list nodes.
		virtual void Read(const char* name); //58F750, raw ENC resource records.
		JE::EncyclopediaEntry* FindEntry(DWORD id); //58F2B0, first matching nonNULL borrowed entry.
		std::list<JE::EncyclopediaEntry*> entries; // Original+4, owning.
	private:
		DWORD ReadEncyclopediaDword(const BYTE*& cursor); //5A1E50; nonvirtual, receiver unused.
		const char* ReadEncyclopediaString(const BYTE*& cursor); //5A1E60; nonvirtual.
	};
namespace JE
{
	typedef ::Encyclopedia Encyclopedia;
	class CStore;
	class StoreManager
	{
	public:
		StoreManager(); // 0x5A1FB0.
		~StoreManager(); // 0x5A2120, NONvirtual despite erroneous database filebuf label.
		virtual void Cleanup(); // 0x5A21A0 ->5A1FF0, deletes owned stores before list nodes.
		CStore* GetStore(const char* name); //5A23B0, newly loaded store still returns NULL.
		std::list<CStore*> stores; // Original+4, owning payloads.
	private:
		void ClearStores(); //5A1FF0, shared folded owning-list body; native name unknown.
	};
	namespace Common
	{
		class Polygon
		{
		public:
			Polygon(); // Constructor portion of59A340; actual vector initialized empty.
			virtual ~Polygon(); // 0x599ED0, actual vector lifetime.
			BYTE Contains(const D3DXVECTOR3& position) const; //5B8A20, nonvirtual.
			Polygon& operator=(const Polygon& source); //5B8BD0; clear before reading source.
			void Translate(const D3DXVECTOR3& translation); //5B8B90; sequential borrowed XYZ.
			void ReleasePoints(); // Explicit vector-release factor, not a separate original routine.
			std::vector<D3DXVECTOR3> points; // Original+4; append574820 has12-byte elements.
		};
	}
	struct WorldMapLocation
	{
		WorldMapLocation(); // 0x59A0C0, destructor599DF0: normal three-string lifetime.
		~WorldMapLocation(); //599DF0, ordinary reverse three-string lifetime.
		void GetCoordinates(LONG& x, LONG& y) const; //599D70; sequential required outputs.
		DWORD GetLabelReference() const; //599D90; flags bit2 chooses values4/6.
		std::string& GetTextureName(std::string& output) const; //59A1A0; existing output string.
		std::string& GetOnEnterScript(std::string& output) const; //59A1C0, assigns value58 to caller's string.
		DWORD values[8];
		std::string value20, value3C, value58;
	};
	struct WorldMapRegion
	{
		WorldMapRegion(); // 0x59A340.
		~WorldMapRegion(); // 0x599F20, releases points before Polygon destruction.
		void AddPoint(LONG x, LONG y); // 0x59A410, stores(float(x),0,float(y)).
		DWORD value0, value4, value8;
		std::string valueC;
		Common::Polygon polygon28;
	};
	class WorldMap
	{
	public:
		WorldMap(); // 0x599F90.
		virtual ~WorldMap(); // 0x59A250, locations before regions.
		virtual void Cleanup(); // 0x59A330, retains strings/scalar settings.
		virtual void Read(const char* name); //0x59A440, raw WorldMap resource reader.
		WorldMapLocation* FindLocation(DWORD id); //599DB0, borrowed first matching area.
		void ClearLocations(); // 0x599E60.
		void ClearRegions(); // 0x59A1E0.
		std::string resourceName; // Original+4.
		DWORD value20;
		BYTE flag24; // Initial1, three padding bytes remain unwritten.
		DWORD value28, value2C;
		std::string value30, value4C;
		std::list<WorldMapLocation*> locations; // Original+68, owning.
		std::list<WorldMapRegion*> regions; // Original+74, owning.
	private:
		DWORD ReadWorldMapDword(const BYTE*& cursor); //5A1E50; nonvirtual, receiver unused.
		const char* ReadWorldMapString(const BYTE*& cursor); //5A1E60; nonvirtual.
	};
}
