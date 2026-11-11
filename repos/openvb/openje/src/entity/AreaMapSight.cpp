#include "MapFog.hpp"
#include <math.h>

namespace JE
{
	void AreaMap::RebuildHeights()
	{
		derivedHeights = heights;
		for (std::map<LONG,AreaMapLayer>::const_iterator layer = layers.begin(); layer != layers.end(); ++layer)
		{
			if (layer->second.disabled) continue;
			for (std::list<AreaMapHeight>::const_iterator cell = layer->second.cells.begin(); cell != layer->second.cells.end(); ++cell)
			{
				if (cell->first < 0 || cell->first >= width || cell->row < 0 || cell->row >= height) continue;
				LONG index = cell->first + width * cell->row;
				if (cell->height > heights[index]) derivedHeights[index] = cell->height;
			}
		}
		for (std::map<DWORD,AreaMapRectangle>::const_iterator entry = rectangles.begin(); entry != rectangles.end(); ++entry)
		{
			const AreaMapRectangle& rectangle = entry->second;
			for (LONG row = rectangle.row; row <= rectangle.lastRow; ++row)
				for (LONG first = rectangle.first; first <= rectangle.last; ++first)
				{
					if (first < 0 || first >= width || row < 0 || row >= height) continue;
					LONG index = first + width * row;
					if (rectangle.height > heights[index]) derivedHeights[index] = rectangle.height;
				}
		}
		for (std::map<DWORD,std::list<AreaMapHeight> >::const_iterator entry = heightLists.begin(); entry != heightLists.end(); ++entry)
			for (std::list<AreaMapHeight>::const_iterator cell = entry->second.begin(); cell != entry->second.end(); ++cell)
			{
				if (cell->first < 0 || cell->first >= width || cell->row < 0 || cell->row >= height) continue;
				LONG index = cell->first + width * cell->row;
				// The original compares BASE height in all three phases, rather than
				// the earlier derived value. Later qualifying influences overwrite it.
				if (cell->height > heights[index]) derivedHeights[index] = cell->height;
			}
	}
	float AreaMap::GetHeight(LONG first, LONG row)
	{
		if (first < 0 || first >= width || row < 0 || row >= height) return 0.0f;
		if (heightsDirty)
		{
			RebuildHeights();
			heightsDirty = 0;
		}
		return derivedHeights[first + width * row];
	}
	BYTE AreaMap::TestLOS(LONG fromFirst, LONG fromRow, float fromHeight,
		LONG toFirst, LONG toRow, const float* targetHeight)
	{
		if (fromFirst < 0 || fromFirst >= width || fromRow < 0 || fromRow >= height
			|| toFirst < 0 || toFirst >= width || toRow < 0 || toRow >= height) return 0;
		LONG dx = LONG(DWORD(toFirst) - DWORD(fromFirst));
		LONG dy = LONG(DWORD(toRow) - DWORD(fromRow));
		double absX = fabs(double(dx));
		float absY = float(fabs(double(dy)));
		float stepX = absX >= absY ? float(2 * (toFirst > fromFirst) - 1) : float(dx / absY);
		float stepY = absY >= absX ? float(2 * (toRow > fromRow) - 1) : float(dy / absX);
		LONG steps = LONG(absX > absY ? absX : absY);
		if (steps <= 0) return 1;
		double destination = targetHeight ? *targetHeight : GetHeight(toFirst, toRow);
		float rise = float((destination - fromHeight) / steps);
		float first = float(fromFirst), row = float(fromRow);
		float currentHeight = fromHeight;
		for (LONG step = 0; step < steps; ++step)
		{
			if (step && GetHeight(LONG(first), LONG(row)) > currentHeight) return 0;
			first = float(double(first) + stepX);
			row = float(double(row) + stepY);
			currentHeight = float(double(currentHeight) + rise);
		}
		return 1;
	}
}
