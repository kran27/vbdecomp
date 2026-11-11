#include "MapGrid.hpp"

namespace JE
{
	template<class T> RLECell<T>* RLEGrid<T>::GetCell(LONG first, LONG row)
	{
		RLECell<T>* result = NULL;
		std::list<RLECell<T>*>& cells = rows[row];
		for (typename std::list<RLECell<T>*>::iterator entry = cells.begin();
			entry != cells.end(); ++entry)
		{
			RLECell<T>* cell = *entry;
			if (cell->first <= first && cell->first + cell->count - 1 >= first)
			{
				result = cell;
				break;
			}
		}
		return result;
	}

	template<class T> void RLEGrid<T>::CollectFootprintValues(std::set<T>& output,
		const std::vector<RLESpan*>& spans, LONG fromFirst, LONG fromRow,
		LONG toFirst, LONG toRow)
	{
		(void)fromRow;
		const LONG gridWidth = width, gridHeight = height;
		for (std::vector<RLESpan*>::const_iterator entry = spans.begin();
			entry != spans.end(); ++entry)
		{
			RLESpan* span = *entry;
			if (!span) continue;
			LONG first = span->first - fromFirst + toFirst;
			LONG count = span->count;
			// Both originals use toRow for EVERY span. Neither span.row nor
			// fromRow is read; this differs from the A* neighbor query560C00.
			if (toRow < 0 || toRow >= gridHeight || first >= gridWidth || first + count <= 0)
			{
				continue;
			}
			if (first < 0)
			{
				count += first;
				first = 0;
			}
			else if (first + count >= gridWidth)
			{
				count = gridWidth - first;
			}
			LONG end = first + count;
			while (first < end)
			{
				RLECell<T>* cell = GetCell(first, toRow);
				for (typename std::list<T>::iterator value = cell->values.begin();
					value != cell->values.end(); ++value)
				{
					const T captured = *value;
					output.insert(captured);
				}
				first = cell->first + cell->count;
			}
		}
	}

	template RLECell<BYTE>* RLEGrid<BYTE>::GetCell(LONG, LONG);
	template RLECell<Entity*>* RLEGrid<Entity*>::GetCell(LONG, LONG);
	template void RLEGrid<BYTE>::CollectFootprintValues(std::set<BYTE>&,
		const std::vector<RLESpan*>&, LONG, LONG, LONG, LONG);
	template void RLEGrid<Entity*>::CollectFootprintValues(std::set<Entity*>&,
		const std::vector<RLESpan*>&, LONG, LONG, LONG, LONG);
}
