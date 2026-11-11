#pragma once

#include "SpatialFootprint.hpp"
#include <list>
#include <set>
#include <string>
#include <vector>

namespace JE
{
	namespace Resources { struct ResourceFileHandle; }
	template<class T> struct RLECell
	{
		RLECell(LONG firstValue, LONG countValue) : first(firstValue), count(countValue) {}
		std::list<T> values;
		LONG first, count;
	};

	// Actual nonvirtual24-byte VC7 row owner. Cells own their value-list nodes;
	// entity pointer values are borrowed, never deleted by the grid.
	template<class T> class RLEGrid
	{
	public:
		RLEGrid(LONG width, LONG height); // Entity* specialization57A8A0.
		explicit RLEGrid(const std::string& resource); // BYTE specialization57B2A0.
		~RLEGrid(); // Shared579BB0.
		void Read(Resources::ResourceFileHandle* stream); // BYTE specialization57AA00, thiscall/RET4.
		void AddFootprint(RLEPrimitive<T>* primitive); // Entity*40FF50.
		void RemoveEntity(T value); // Calls original per-row40F070.
		void RemoveFromRow(LONG row, T value); // Entity*40F070.
		RLECell<T>* GetCell(LONG first, LONG row); // Shared560B20, unchecked row; NULL if no run.
		void CollectFootprintValues(std::set<T>& output, const std::vector<RLESpan*>& spans,
			LONG fromFirst, LONG fromRow, LONG toFirst, LONG toRow); // Entity*405F80/BYTE561A90.

		LONG width, height;
		std::vector<std::list<RLECell<T>*> > rows;

	private:
		void AddRange(T value, LONG first, LONG row, LONG count); // Entity*40F7B0.
		RLEGrid(const RLEGrid&);
		RLEGrid& operator=(const RLEGrid&);
	};

	typedef RLEGrid<BYTE> MapGrid;
	typedef RLEGrid<Entity*> EntityGrid;
}
