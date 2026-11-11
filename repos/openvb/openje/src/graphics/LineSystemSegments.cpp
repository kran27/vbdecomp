#include "LineSystem.hpp"
#include <string.h>

LONG Gfx_LineSystem::GetSegmentCount() const //49D260.
{
	return LONG(values108.size());
}

LONG* Gfx_LineSystem::GetSegment(LONG index, float* x1, float* y1, float* z1,
	float* x2, float* y2, float* z2, LONG* r1, LONG* g1, LONG* b1, LONG* a1,
	LONG* r2, LONG* g2, LONG* b2, LONG* a2) const //50E8C0.
{
	//Snapshot all56B before optional stores, including when an output aliases storage.
	std::vector<Gfx_LineSystemValue>::const_iterator selected = values108.begin();
	if (index > 0) selected += index;
	const Gfx_LineSystemValue value(*selected);
	if (x1) memcpy(x1, value.values, sizeof(*x1));
	if (y1) memcpy(y1, value.values + 1, sizeof(*y1));
	if (z1) memcpy(z1, value.values + 2, sizeof(*z1));
	if (x2) memcpy(x2, value.values + 3, sizeof(*x2));
	if (y2) memcpy(y2, value.values + 4, sizeof(*y2));
	if (z2) memcpy(z2, value.values + 5, sizeof(*z2));
	if (r1) memcpy(r1, value.values + 6, sizeof(*r1));
	if (g1) memcpy(g1, value.values + 7, sizeof(*g1));
	if (b1) memcpy(b1, value.values + 8, sizeof(*b1));
	if (a1) memcpy(a1, value.values + 9, sizeof(*a1));
	if (r2) memcpy(r2, value.values + 10, sizeof(*r2));
	if (g2) memcpy(g2, value.values + 11, sizeof(*g2));
	if (b2) memcpy(b2, value.values + 12, sizeof(*b2));
	if (a2) memcpy(a2, value.values + 13, sizeof(*a2));
	return a2;
}

void Gfx_LineSystem::AddSegment(float x1,float y1,float z1,float x2,float y2,float z2,
	LONG red1,LONG green1,LONG blue1,LONG alpha1,
	LONG red2,LONG green2,LONG blue2,LONG alpha2) //50F870.
{
	Gfx_LineSystemValue value;
	memcpy(value.values, &x1, sizeof(x1));
	memcpy(value.values+1, &y1, sizeof(y1));
	memcpy(value.values+2, &z1, sizeof(z1));
	memcpy(value.values+3, &x2, sizeof(x2));
	memcpy(value.values+4, &y2, sizeof(y2));
	memcpy(value.values+5, &z2, sizeof(z2));
	value.values[6]=DWORD(red1); value.values[7]=DWORD(green1);
	value.values[8]=DWORD(blue1); value.values[9]=DWORD(alpha1);
	value.values[10]=DWORD(red2); value.values[11]=DWORD(green2);
	value.values[12]=DWORD(blue2); value.values[13]=DWORD(alpha2);
	values108.push_back(value);
	enabled=1;
}
