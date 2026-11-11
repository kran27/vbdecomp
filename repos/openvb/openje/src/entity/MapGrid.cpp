#include "MapGrid.hpp"
#include "resources/ResourceSystem.hpp"
#include <algorithm>

namespace JE
{
	template<class T> RLEGrid<T>::RLEGrid(LONG widthValue, LONG heightValue)
	{
		std::list<RLECell<T>*> emptyRow;
		width = widthValue;
		height = heightValue;
		rows.resize(height, emptyRow);
		for (LONG row = 0; row < height; ++row)
		{
			rows[row].push_back(new RLECell<T>(0, width));
		}
	}

	template<> RLEGrid<BYTE>::RLEGrid(const std::string& resource)
	{
		Resources::ResourceFileHandle* stream = Resources::g_pResourceSystem->OpenStream(resource.c_str(), 1);
		if (!stream)
		{
			width = height = 0;
			return;
		}
		Read(stream);
		Resources::g_pResourceSystem->Release(stream, 0);
	}

	template<> void RLEGrid<BYTE>::Read(Resources::ResourceFileHandle* stream)
	{
		// Original57AA00 deliberately ignores each read result. Truncated input
		// leaves locals unwritten; malformed nonpositive run lengths can stall.
		LONG fileWidth, fileHeight;
		Resources::g_pResourceSystem->Read(&fileWidth, 4, stream);
		Resources::g_pResourceSystem->Read(&fileHeight, 4, stream);
		width = fileWidth;
		height = fileHeight;
		std::list<RLECell<BYTE>*> emptyRow;
		rows.resize(height, emptyRow);
		for (LONG fileRow = 0; fileRow < fileHeight; ++fileRow)
		{
			LONG runStartColumn = 0;
			std::list<RLECell<BYTE>*>& cells = rows[fileHeight - fileRow - 1];
			while (runStartColumn < fileWidth)
			{
				LONG runLength;
				BYTE runValue;
				Resources::g_pResourceSystem->Read(&runLength, 4, stream);
				Resources::g_pResourceSystem->Read(&runValue, 1, stream);
				RLECell<BYTE>* runCell = new RLECell<BYTE>(runStartColumn, runLength);
				runCell->values.push_back(runValue);
				cells.push_back(runCell);
				runStartColumn += runLength;
			}
		}
	}

	template<class T> RLEGrid<T>::~RLEGrid()
	{
		for (LONG row = 0; row < height; ++row)
		{
			std::list<RLECell<T>*>& cells = rows[row];
			for (typename std::list<RLECell<T>*>::iterator cell = cells.begin();
				cell != cells.end(); ++cell)
			{
				delete *cell;
			}
		}
	}

	template<class T> void RLEGrid<T>::AddFootprint(RLEPrimitive<T>* primitive)
	{
		for (DWORD index = 0; index < primitive->spans.size(); ++index)
		{
			RLESpan* span = primitive->spans[index];
			if (!span) continue;
			LONG row = span->row, first = span->first, count = span->count;
			if (row < 0 || row >= height || first >= width || first + count <= 0) continue;
			if (first < 0)
			{
				count += first;
				first = 0;
			}
			else if (first + count >= width)
			{
				count = width - first;
			}
			AddRange(primitive->value, first, row, count);
		}
	}

	template<class T> void RLEGrid<T>::AddRange(T value, LONG first, LONG row, LONG count)
	{
		typedef typename std::list<RLECell<T>*>::iterator Iterator;
		std::list<RLECell<T>*>& cells = rows[row];
		RLECell<T>* pending = new RLECell<T>(first, count);
		pending->values.push_back(value);
		for (;;)
		{
			Iterator found = cells.begin();
			while (!((*found)->first <= pending->first &&
				(*found)->first + (*found)->count - 1 >= pending->first)) ++found;
			RLECell<T>* existing = *found;
			LONG oldFirst = existing->first, oldCount = existing->count;
			if (pending->first == oldFirst)
			{
				if (pending->count == oldCount)
				{
					existing->values.insert(existing->values.end(), pending->values.begin(), pending->values.end());
					delete pending;
					return;
				}
				if (pending->count < oldCount)
				{
					existing->first += pending->count;
					existing->count -= pending->count;
					pending->values.insert(pending->values.end(), existing->values.begin(), existing->values.end());
					cells.insert(found, pending);
					return;
				}
				existing->values.insert(existing->values.end(), pending->values.begin(), pending->values.end());
				pending->first += oldCount;
				pending->count -= oldCount;
				continue;
			}

			LONG oldEnd = oldFirst + oldCount;
			LONG pendingEnd = pending->first + pending->count;
			if (pendingEnd == oldEnd)
			{
				existing->count = pending->first - oldFirst;
				pending->values.insert(pending->values.end(), existing->values.begin(), existing->values.end());
				cells.insert(++found, pending);
				return;
			}
			if (pendingEnd < oldEnd)
			{
				existing->count = pending->first - oldFirst;
				pending->values.insert(pending->values.end(), existing->values.begin(), existing->values.end());
				RLECell<T>* suffix = new RLECell<T>(pendingEnd, oldEnd - pendingEnd);
				suffix->values.insert(suffix->values.end(), existing->values.begin(), existing->values.end());
				Iterator next = cells.insert(++found, suffix);
				cells.insert(next, pending);
				return;
			}
			existing->count = pending->first - oldFirst;
			LONG remainder = pending->count - (oldEnd - pending->first);
			pending->count = oldEnd - pending->first;
			cells.insert(++found, pending);
			RLECell<T>* suffix = new RLECell<T>(pending->first + pending->count, remainder);
			suffix->values.insert(suffix->values.end(), pending->values.begin(), pending->values.end());
			pending->values.insert(pending->values.end(), existing->values.begin(), existing->values.end());
			pending = suffix;
		}
	}

	template<class T> void RLEGrid<T>::RemoveEntity(T value)
	{
		for (LONG row = 0; row < height; ++row) RemoveFromRow(row, value);
	}

	template<class T> void RLEGrid<T>::RemoveFromRow(LONG row, T value)
	{
		typedef typename std::list<RLECell<T>*>::iterator Iterator;
		std::list<RLECell<T>*>& cells = rows[row];
		BYTE removed = 0;
		for (Iterator cell = cells.begin(); cell != cells.end(); ++cell)
		{
			typename std::list<T>::iterator found = std::find((*cell)->values.begin(), (*cell)->values.end(), value);
			if (found != (*cell)->values.end())
			{
				(*cell)->values.remove(value); //40E780 removes all matching payloads.
				removed = 1;
			}
		}
		if (!removed) return;
		for (Iterator cell = cells.begin(); cell != cells.end(); ++cell)
		{
			Iterator previous = cell;
			--previous;
			RLECell<T>* current = *cell;
			Iterator next = cell;
			++next; //40F10E: hold the next node before either neighboring deletion.
			if (previous != cells.end())
			{
				RLECell<T>* merged = *previous;
				if (merged->values.size() == current->values.size())
				{
					typename std::list<T>::const_iterator member = merged->values.begin();
					for (; member != merged->values.end(); ++member)
						if (std::find(current->values.begin(), current->values.end(), *member) == current->values.end()) break;
					if (member == merged->values.end())
					{
						current->count += merged->count;
						current->first = merged->first;
						cells.remove(merged);
						delete merged;
					}
				}
			}
			if (next != cells.end())
			{
				RLECell<T>* merged = *next;
				if (merged->values.size() == current->values.size())
				{
					typename std::list<T>::const_iterator member = merged->values.begin();
					for (; member != merged->values.end(); ++member)
						if (std::find(current->values.begin(), current->values.end(), *member) == current->values.end()) break;
					if (member == merged->values.end())
					{
						current->count += merged->count;
						cells.remove(merged);
						delete merged;
					}
				}
			}
		}
	}

	// Emit the actual used specializations only. In particular, the original
	// entity grid has no resource-file constructor; do not request its emission.
	template RLEGrid<BYTE>::~RLEGrid();
	template RLEGrid<Entity*>::RLEGrid(LONG, LONG);
	template RLEGrid<Entity*>::~RLEGrid();
	template void RLEGrid<Entity*>::AddFootprint(RLEPrimitive<Entity*>*);
	template void RLEGrid<Entity*>::RemoveEntity(Entity*);
}
