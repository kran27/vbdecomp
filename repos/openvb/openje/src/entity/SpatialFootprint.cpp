#include "SpatialFootprint.hpp"
#include "startup/MessageLoop.hpp"
#include <stdlib.h>
#include <limits.h>
#include <stddef.h>

namespace JE
{
	template<class T> RLEPrimitive<T>::RLEPrimitive(const T& input)
	{
		//40B665..40B684: spans constructs before the payload copy and clock read.
		value = input;
		updatedAt = F3::GetElapsedTimeMS();
	}

	template<class T> RLEPrimitive<T>::~RLEPrimitive()
	{
		while (!spans.empty())
		{
			RLESpan* span = spans.back();
			spans.pop_back();
			delete span;
		}
	}

	template<class T> DWORD RLEPrimitive<T>::Touch() { return updatedAt = F3::GetElapsedTimeMS(); }

	template<class T> void RLEPrimitive<T>::Include(LONG first, LONG row, LONG index)
	{
		RLESpan* span = spans[index]; //Original requires the allocated row domain unchecked.
		if (!span)
		{
			RLESpan* created = new RLESpan;
			created->row = row;
			created->first = first;
			created->count = 1;
			spans[index] = created; //40A1DE reloads the live vector after allocation.
		}
		else if (first < span->first)
		{
			span->count += span->first - first;
			span->first = first;
		}
		else if (first > span->first && first >= span->first + span->count)
			span->count = first - span->first + 1;
	}

	template<class T> RLECircle<T>::RLECircle(LONG r, LONG x, LONG y, const T& value)
		: RLEPrimitive<T>(value), radius(r), first(x), row(y)
	{
		Rasterize();
	}

	template<class T> void RLECircle<T>::PrepareRows() { this->spans.resize(2 * radius + 1, NULL); }

	template<class T> void RLECircle<T>::Rasterize()
	{
		PrepareRows();
		LONG y = radius, x = 0, decision = 1 - y, increment = 3, diagonal = 5 - 2 * y;
		this->Include(0, y, y + radius);
		this->Include(0, y, y + radius);
		this->Include(0, -y, radius - y);
		this->Include(0, -y, radius - y);
		this->Include(y, 0, radius);
		this->Include(-y, 0, radius);
		this->Include(y, 0, radius);
		this->Include(-y, 0, radius);
		if (y > 0)
		{
			do
			{
				if (decision >= 0)
				{
					decision += diagonal;
					++x;
					diagonal += 4;
					--y;
					increment += 2;
				}
				else
				{
					decision += increment;
					++x;
					increment += 2;
					diagonal += 2;
				}
				this->Include(x, y, y + radius);
				this->Include(-x, y, y + radius);
				this->Include(x, -y, radius - y);
				this->Include(-x, -y, radius - y);
				this->Include(y, x, x + radius);
				this->Include(-y, x, x + radius);
				this->Include(y, -x, radius - x);
				this->Include(-y, -x, radius - x);
			} while (y > x);
		}
		for (size_t i = 0; i < this->spans.size(); ++i)
		{
			this->spans[i]->row += row;
			this->spans[i]->first += first;
		}
	}

	template<class T> void RLECircle<T>::Move(LONG x, LONG y)
	{
		for (std::vector<RLESpan*>::iterator i = this->spans.begin(); i != this->spans.end(); ++i)
		{
			RLESpan* span = *i;
			if (span)
			{
				span->row += y - row;
				span->first += x - first;
			}
		}
		first = x;
		row = y;
	}
	
	template<class T> BYTE RLECircle<T>::Matches(LONG x, LONG y, float) const { return MatchesPosition(x,y); }
	template<class T> BYTE RLECircle<T>::MatchesPosition(LONG x, LONG y) const { return x == first && y == row; }
	template<class T> BYTE RLECircle<T>::MatchesAngle(float) const { return false; }

	template<class T> RLEConvexQuad<T>::RLEConvexQuad(LONG first0, LONG row0, LONG first1, LONG row1,
		LONG first2, LONG row2, LONG first3, LONG row3, float rotation, const T& value)
		: RLEPrimitive<T>(value), angle(rotation), minimumFirst(LONG_MAX), minimumRow(LONG_MAX),
		maximumFirst(LONG_MIN), maximumRow(LONG_MIN)
	{
		points[0][0] = first0; points[0][1] = row0;
		points[1][0] = first1; points[1][1] = row1;
		points[2][0] = first2; points[2][1] = row2;
		points[3][0] = first3; points[3][1] = row3;
		for (int i = 0; i < 4; ++i)
		{
			if (points[i][0] < minimumFirst) minimumFirst = points[i][0];
			if (points[i][0] > maximumFirst) maximumFirst = points[i][0];
			if (points[i][1] < minimumRow) minimumRow = points[i][1];
			if (points[i][1] > maximumRow) maximumRow = points[i][1];
		}
		first = (maximumFirst - minimumFirst) / 2 + minimumFirst;
		row = (maximumRow - minimumRow) / 2 + minimumRow;
		for (int i = 0; i < 4; ++i)
		{
			points[i][0] -= first;
			points[i][1] -= row;
		}
		Rasterize();
	}

	template<class T> void RLEConvexQuad<T>::PrepareRows()
	{
		// Original40CC63 subtracts minimumFirst(+48), not minimumRow(+4C).
		this->spans.resize(maximumRow - minimumFirst + 1, NULL);
	}

	template<class T> void RLEConvexQuad<T>::Rasterize()
	{
		PrepareRows();
		for (int i = 0; i < 4; ++i)
		{
			const int next = i == 3 ? 0 : i + 1;
			LONG x = first + points[i][0], y = row + points[i][1];
			LONG endX = first + points[next][0], endY = row + points[next][1];
			LONG dx = labs(endX - x), dy = labs(endY - y);
			LONG increment, decision, diagonal, steps, stepX, stepY;
			if (dx < dy)
			{
				increment = 2 * dx;
				decision = increment - dy;
				diagonal = 2 * (dx - dy);
				steps = dy + 1;
				stepX = 0;
				stepY = 1;
			}
			else
			{
				increment = 2 * dy;
				decision = increment - dx;
				diagonal = 2 * (dy - dx);
				steps = dx + 1;
				stepX = 1;
				stepY = 0;
			}
			LONG diagonalX = 1, diagonalY = 1;
			if (x > endX) { stepX = -stepX; diagonalX = -1; }
			if (y > endY) { stepY = -stepY; diagonalY = -1; }
			for (; steps > 0; --steps)
			{
				this->Include(x, y, y - minimumRow);
				if (decision >= 0)
				{
					x += diagonalX;
					y += diagonalY;
					decision += diagonal;
				}
				else
				{
					x += stepX;
					y += stepY;
					decision += increment;
				}
			}
		}
	}

	template<class T> void RLEConvexQuad<T>::Move(LONG x, LONG y)
	{
		for (std::vector<RLESpan*>::iterator i = this->spans.begin(); i != this->spans.end(); ++i)
		{
			RLESpan* span = *i;
			if (span)
			{
				span->row += y - row;
				span->first += x - first;
			}
		}
		first = x;
		row = y;
	}
	template<class T> BYTE RLEConvexQuad<T>::Matches(LONG x, LONG y, float a) const
	{
		return x == first && y == row && a == angle;
	}
	template<class T> BYTE RLEConvexQuad<T>::MatchesPosition(LONG x, LONG y) const { return Matches(x,y,0.0f); }
	template<class T> BYTE RLEConvexQuad<T>::MatchesAngle(float a) const { return a == angle; }

	template class RLEPrimitive<Entity*>;
	template class RLECircle<Entity*>;
	template class RLEConvexQuad<Entity*>;

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef RLEPrimitive<Entity*> PrimitiveLayout;
	typedef RLECircle<Entity*> CircleLayout;
	typedef RLEConvexQuad<Entity*> QuadLayout;
	typedef char SpanExtent[(sizeof(RLESpan) == 12) ? 1 : -1];
	typedef char PrimitiveExtent[(sizeof(PrimitiveLayout) == 0x1C) ? 1 : -1];
	typedef char PrimitiveValue[(offsetof(PrimitiveLayout, value) == 4) ? 1 : -1];
	typedef char PrimitiveClock[(offsetof(PrimitiveLayout, updatedAt) == 8) ? 1 : -1];
	typedef char PrimitiveSpans[(offsetof(PrimitiveLayout, spans) == 0xC) ? 1 : -1];
	typedef char CircleExtent[(sizeof(CircleLayout) == 0x28) ? 1 : -1];
	typedef char CircleRadius[(offsetof(CircleLayout, radius) == 0x1C) ? 1 : -1];
	typedef char CircleFirst[(offsetof(CircleLayout, first) == 0x20) ? 1 : -1];
	typedef char CircleRow[(offsetof(CircleLayout, row) == 0x24) ? 1 : -1];
	typedef char QuadExtent[(sizeof(QuadLayout) == 0x58) ? 1 : -1];
	typedef char QuadFirst[(offsetof(QuadLayout, first) == 0x1C) ? 1 : -1];
	typedef char QuadRow[(offsetof(QuadLayout, row) == 0x20) ? 1 : -1];
	typedef char QuadAngle[(offsetof(QuadLayout, angle) == 0x24) ? 1 : -1];
	typedef char QuadPoints[(offsetof(QuadLayout, points) == 0x28) ? 1 : -1];
	typedef char QuadMinimumFirst[(offsetof(QuadLayout, minimumFirst) == 0x48) ? 1 : -1];
	typedef char QuadMinimumRow[(offsetof(QuadLayout, minimumRow) == 0x4C) ? 1 : -1];
	typedef char QuadMaximumFirst[(offsetof(QuadLayout, maximumFirst) == 0x50) ? 1 : -1];
	typedef char QuadMaximumRow[(offsetof(QuadLayout, maximumRow) == 0x54) ? 1 : -1];
#endif
}
