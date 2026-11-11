#pragma once

#include "ModelMesh.hpp"
#include "PacketData.hpp"
#include <d3dx8.h>

class G3D_Node;
class G3D_WaterTile;
class Gfx_ModelNode;
class Gfx_Viewport;
class Gfx_Water;

struct Gfx_WaterVertex32
{
	// Original arrays are POD storage, unwritten until geometry generation.
	float x, y, z;
	DWORD color;
	float u, v, lightU, lightV;
};

struct Gfx_WaterVertex44
{
	float x, y, z, nx, ny, nz;
	DWORD color;
	float u, v, lightU, lightV;
};

class Gfx_WaterTile : public Gfx_NodeAttribute
{
public:
	Gfx_WaterTile(); //512010.
	virtual ~Gfx_WaterTile(); //5125C0.
	virtual void Update(float time); //511570.
	Gfx_WaterTile* Clone(Gfx_ModelNode* owner) const; //49FF19..4A00AD.
	void LoadSource(G3D_Node* node, const G3D_WaterTile& source, BYTE resources); //528196..5286FE.
	Gfx_Water* GetWater(); //50FDC0/51DAF0.
	void Rebuild(); //512330.
	void SetTexture(DWORD handle, BYTE owned); //511430.
	void SetLightmap(DWORD handle, BYTE owned); //5114A0.
	void SetTextureUV(float u, float v, float width, float height); //511EF0.
	void SetSurfaceUV(float u, float v, float width, float height); //50FDE0.
	//50FD70 transports physical component words; original pointee spelling is unknown.
	BYTE GetCornerColor(LONG corner, DWORD* red, DWORD* green, DWORD* blue) const;
	BYTE SetCornerColor(DWORD corner, DWORD red, DWORD green, DWORD blue); //50FD30 ignores colors.
	void Draw(Gfx_Viewport* viewport); //512B40.
	virtual BYTE AddLight(Gfx_Light* light); //5127B0.
	virtual void RemoveLight(Gfx_Light* light); //511F70.
	virtual void ClearLights(); //511FD0.
	virtual void FinishLights(); //512830.

	// Ordinary original members in observed constructor/lifetime order. Two
	// untyped regions (original CC..D7 and170..173) remain unrepresented; the
	// current class is16B short of258h. This is not a complete layout match.
	Gfx_MeshData mesh; //Original+24,108-byte owned mesh.
	Gfx_MeshInstance* instances; //+90, owned array.
	Gfx_PacketData* packet; //+94, owned polymorphic packet.
	D3DXMATRIX* transforms; //+98, owned array.
	LONG instanceCount; //+9C;0 initially, Rebuild publishes1.
	Gfx_WaterVertex44* vertices44; //+A0.
	Gfx_WaterVertex32* vertices32; //+A4.
	DWORD vertexCount; //+A8.
	WORD* indices; //+AC.
	DWORD indexCount, primitiveCount; //+B0/+B4.
	BYTE dirty, triangleList; //+B8/+B9.
	DWORD red, green, blue, alpha; //+BC..C8.
	// Original CC..D7 receives three zero words; source types unresolved.
	std::string waterName; //Original+D8, complete source-node path.
	DWORD texture; //Original+F4.
	BYTE ownsTexture; //+F8.
	std::string lightmapName; //+FC, stored name passed by511EF0.
	LONG columns, rows, offsetX, offsetZ; //+118..124.
	DWORD lightmap; //+128.
	BYTE ownsLightmap; //+12C.
	D3DXMATRIX localTransform; //+130; constructor writes full identity.
	// Original170..173 is unwritten; no invented scalar or padding.
	float cornerColors[4][3]; //+174..1A3.
	float extentX, extentZ; //+1A4/+1A8.
	float textureWidth, textureHeight, originalTextureWidth, originalTextureHeight; //+1AC..1B8.
	Gfx_Water* directWater; //+1BC.
	float surfaceU, surfaceV, surfaceWidth, surfaceHeight; //+1C0..1CC.
	float lightU, lightV, lightWidth, lightHeight; //+1D0..1DC.
	DWORD mode28; //+1E0, original material mode24/mode28 input.
	D3DXMATRIX extraTransform; //+1E4.
	BYTE flag224, flag225, flag226;
	Gfx_LightList lights; //+228, actual three-word VC7 list.
	float* lastTime; //+234, individually owned timestamp.
	float accumulatedTime; //+238.
	float value23C; //+23C, update sampling interval.
	BYTE animateAlpha, normalizeAlphaTime; //+240/+241.
	LONG alphaEnd;
	DWORD alphaWaveform;
	float alphaFrequency, alphaPhase, alphaDuration; //+24C..254.

private:
	void BuildIndices(const char* lightmapName); //50FE10; borrowed name is unused; original mask/spiral policies.
	void BuildPacket(); //512850.
	Gfx_WaterTile(const Gfx_WaterTile&);
	Gfx_WaterTile& operator=(const Gfx_WaterTile&);
};
