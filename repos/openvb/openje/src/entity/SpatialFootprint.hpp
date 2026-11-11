#pragma once

#include <windows.h>
#include <vector>

namespace JE
{
	class Entity;
	struct RLESpan
	{
		LONG row, first, count;
	};

	// Original RLEPrimitive<Entity*> family; real VC7 containers own span slots.
	// Constructors copy the referenced payload before sampling actual56AF50.
	template<class T> class RLEPrimitive
	{
	public:
		RLEPrimitive(const T& value); //40B640, one payload-reference argument.
		virtual ~RLEPrimitive(); // 40ABA0, also used by derived destruction40CC50.
		virtual void Rasterize() = 0;
		virtual DWORD Touch(); //409BC0,+08, samples56AF50 and returns the clock.
		virtual void Move(LONG first, LONG row) = 0;
		virtual BYTE Matches(LONG first, LONG row, float angle) const = 0;
		virtual BYTE MatchesPosition(LONG first, LONG row) const = 0;
		virtual BYTE MatchesAngle(float angle) const = 0;
		T value;
		DWORD updatedAt;
		std::vector<RLESpan*> spans;
	protected:
		virtual void PrepareRows() = 0;
		void Include(LONG first, LONG row, LONG index); //40A1B0 and inlined copies.
	private:
		RLEPrimitive(const RLEPrimitive&);
		RLEPrimitive& operator=(const RLEPrimitive&);
	};

	template<class T> class RLECircle : public RLEPrimitive<T>
	{
	public:
		RLECircle(LONG radius, LONG first, LONG row, const T& value); //54FEA0,RET16.
		virtual void Rasterize(); // 54B770.
		virtual void Move(LONG first, LONG row); // 54FF50.
		virtual BYTE Matches(LONG first, LONG row, float angle) const; // 54FF10.
		virtual BYTE MatchesPosition(LONG first, LONG row) const; // 54FF30.
		virtual BYTE MatchesAngle(float angle) const; // Actual591690, alwaysfalse.
		LONG radius, first, row;
	protected:
		virtual void PrepareRows(); // 54FFD0.
	};

	template<class T> class RLEConvexQuad : public RLEPrimitive<T>
	{
	public:
		RLEConvexQuad(LONG first0, LONG row0, LONG first1, LONG row1,
			LONG first2, LONG row2, LONG first3, LONG row3, float angle, const T& value); //40C9B0,RET40.
		virtual void Rasterize(); // 40A4B0, original integer edge/span construction.
		virtual void Move(LONG first, LONG row); // 40CBF0.
		virtual BYTE Matches(LONG first, LONG row, float angle) const; // 40CB80.
		virtual BYTE MatchesPosition(LONG first, LONG row) const; // 40CBB0, comparesangle0.
		virtual BYTE MatchesAngle(float angle) const; // 40CBD0.
		LONG first, row;
		float angle;
		LONG points[4][2], minimumFirst, minimumRow, maximumFirst, maximumRow;
	protected:
		virtual void PrepareRows(); // 40CC60; retains original maximumRow-minimumFirst+1.
	};
}
