#pragma once

#include <d3dx8.h>
#include <list>
#include <map>

class Gfx_Light;
typedef std::list<Gfx_Light*> Gfx_LightList;

// Actual RTTI identities. The base owns the complete80-byte recovered state;
// the derived class adds no storage. Real containers own nodes and borrow lights.
class Gfx_BasePacketData
{
public:
	explicit Gfx_BasePacketData(DWORD value); // 0x4B7990; normal container lifetime.
	virtual ~Gfx_BasePacketData() = 0; // 0x422950; actual pure-virtual base table.
	DWORD LightCount(DWORD primitiveID); // 0x4B74A0; find-only, returns list size.
	Gfx_LightList* FindLights(DWORD primitiveID); // 0x4B74D0; borrowed list or NULL.
	void ClearLights(); //4B76C0, borrowed nodes only; perPrimitiveLights unchanged.
	BYTE AddLight(Gfx_Light* light, DWORD primitiveID); //4B8050, real maximum-light cap.
	BYTE AddLight(Gfx_Light* light); //4B7660, object-wide borrowed list.
	void RemoveLight(Gfx_Light* light); //4B73A0, first matching entry in each list.
	void FinishLights(); //4B7E70, descending signed light+44 order.

	BYTE ambientOverride; // +04.
	D3DCOLOR ambientColor; // +08; padding+05..07 remains uninterpreted.
	DWORD unknown0C;
	DWORD transformEnabled; // +10; tested as a full word.
	const D3DXMATRIX* transform; // +14; borrowed matrix.
	D3DXVECTOR3 scale; // +18.
	D3DXVECTOR4 extraLight; // +24; four raw floats passed to the light API.
	Gfx_LightList lights; // +34; owns nodes, borrows light pointers.
	BYTE perPrimitiveLights; // +40; padding+41..43 remains uninterpreted.
	std::map<DWORD, Gfx_LightList> primitiveLights; // +44; owns lists/nodes only.

private:
	Gfx_BasePacketData(const Gfx_BasePacketData&);
	Gfx_BasePacketData& operator=(const Gfx_BasePacketData&);
};

class Gfx_PacketData : public Gfx_BasePacketData
{
public:
	Gfx_PacketData(); // Factory0x51C870 supplies base argument0.
	virtual ~Gfx_PacketData(); // 0x422A10.
};

inline Gfx_PacketData::Gfx_PacketData()
	: Gfx_BasePacketData(0)
{
}

// 0x51C870: cdecl allocation factory; the original ignores its mesh word.
Gfx_PacketData* Gfx_CreatePacketData(DWORD meshWord);
