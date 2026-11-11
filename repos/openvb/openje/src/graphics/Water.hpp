#pragma once

#include "ModelMesh.hpp"
#include <list>
#include <stddef.h>

struct Gfx_WaterSample
{
	float height, velocity; // Original+B4 array, eight bytes per sample.
};

struct Gfx_WaterRefraction
{
	float x, y;
	DWORD color;
};

struct Gfx_WaterTexturePatch
{
	Gfx_WaterTexturePatch() {} //Preserve default strings and unwritten coordinates.
	Gfx_WaterTexturePatch(LONG x, LONG z, const std::string& texture, const std::string& tile);
	LONG x, z;
	std::string textureName, tileName;
};

// Actual shared water simulation owner51E550. Samples and the optional mask are
// raw owned arrays; the texture-patch list uses the supplied VC7 implementation.
class Gfx_Water : public Gfx_NodeAttribute
{
public:
	Gfx_Water();
	virtual ~Gfx_Water(); //51E770, registry removal belongs to the caller.
	virtual void Update(float elapsed); //51CE00.
	LONG SampleIndex(LONG x, LONG z) const; //51C8E0, retains its unusual byte test.
	void AddSample(LONG x, LONG z, float amount); //51C990.
	void AddRipple(LONG x, LONG z, float amount); //51C9E0.
	void AddLine(LONG x, LONG z, float amount, float directionX, float directionZ); //51CAB0.
	void PrepareSamples(); //51D840; incidental AL is not a success result.
	void Reset(); //51CC60; actual sample/timing reset, shared in original4EDCE0 scope.
	void SetTexture(DWORD handle, BYTE owned); //51D9C0.
	void SetRegistryName(std::string value); //49EAC0; actual by-value string lifetime.
	void SetTextureName(std::string value); //49EB20.
	void SetSecondaryTextureName(std::string value); //49EB80.
	void AddTexturePatch(LONG x, LONG z, const std::string& texture, const std::string& tile); //49F5B0.
	void SetRippleRange(D3DXVECTOR2 minimumLocation, D3DXVECTOR2 maximumLocation,
		float minimumAmount, float maximumAmount, float minimumInterval, float maximumInterval); //51CCC0, RET32.
	void SetLineRange(D3DXVECTOR2 minimumOrigin, D3DXVECTOR2 maximumOrigin,
		D3DXVECTOR2 minimumDirection, D3DXVECTOR2 maximumDirection,
		float minimumAmount, float maximumAmount, float minimumInterval, float maximumInterval); //51CD20, RET48.
	void SetOrbitRange(float minimumRadius, float maximumRadius, float minimumSpeed,
		float maximumSpeed, float minimumAmount, float maximumAmount,
		float minimumInterval, float maximumInterval); //51CDA0, RET32.

	DWORD value24, value28, value2C;
	DWORD textureHandle; BYTE ownsTexture; //+30/+34; ctor leaves ownsTexture unwritten.
	LONG sampleWidth, sampleHeight; //+38/+3C.
	std::string textureName; //+40, setter49EB20.
	float cellSize; //+5C.
	std::string secondaryTextureName; //+60, setter49EB80.
	std::string registryName; //+7C, setter49EAC0.
	float value98, amplitude, inverseWidth, inverseHeight, valueA8, valueAC; //+98..AC.
	float meanHeight; //+B0, first written by Update.
	Gfx_WaterSample* samples; //+B4 owned array, eight bytes per sample.
	Gfx_WaterRefraction* refraction; //+B8 borrowed global lookup.
	BYTE flagBC;
	BYTE* sampleMask; //+C0 owned bit rows, optional NULL.
	LONG tilesX, tilesZ; //+C4/+C8, cells per source tile.
	std::list<Gfx_WaterTexturePatch> texturePatches; //+CC, values own only strings.
	BYTE forceUpdate; float elapsedTime, updateInterval; //+D8/+DC/+E0.
	DWORD disturbanceMode; //+E4.
	D3DXVECTOR2 rippleMinimumLocation, rippleMaximumLocation; //+E8/+F0, real8B pairs.
	float rippleMinimumAmount, rippleMaximumAmount, rippleMinimumInterval, rippleMaximumInterval;
	float rippleNext, rippleTime; //+108/+10C.
	D3DXVECTOR2 lineMinimumOrigin, lineMaximumOrigin, lineMinimumDirection, lineMaximumDirection; //+110..12F.
	float lineMinimumAmount, lineMaximumAmount, lineMinimumInterval, lineMaximumInterval;
	float lineNext, lineTime; //+140/+144.
	float orbitRange[8], orbitNext, orbitTime, orbitAngle; //+148..170.
	BYTE flag174;
private:
	Gfx_Water(const Gfx_Water&);
	Gfx_Water& operator=(const Gfx_Water&);
};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
#define F3_WATER_OFFSET(member, value) typedef char F3_Water_##member[(offsetof(Gfx_Water, member) == value) ? 1 : -1]
typedef char F3_WaterSample_Size[(sizeof(Gfx_WaterSample) == 8) ? 1 : -1];
typedef char F3_WaterRefraction_Size[(sizeof(Gfx_WaterRefraction) == 12) ? 1 : -1];
typedef char F3_Water_Size[(sizeof(Gfx_Water) == 0x178) ? 1 : -1];
F3_WATER_OFFSET(value24, 0x24);
F3_WATER_OFFSET(textureHandle, 0x30);
F3_WATER_OFFSET(ownsTexture, 0x34);
F3_WATER_OFFSET(sampleWidth, 0x38);
F3_WATER_OFFSET(sampleHeight, 0x3C);
F3_WATER_OFFSET(textureName, 0x40);
F3_WATER_OFFSET(cellSize, 0x5C);
F3_WATER_OFFSET(secondaryTextureName, 0x60);
F3_WATER_OFFSET(registryName, 0x7C);
F3_WATER_OFFSET(value98, 0x98);
F3_WATER_OFFSET(amplitude, 0x9C);
F3_WATER_OFFSET(inverseWidth, 0xA0);
F3_WATER_OFFSET(inverseHeight, 0xA4);
F3_WATER_OFFSET(meanHeight, 0xB0);
F3_WATER_OFFSET(samples, 0xB4);
F3_WATER_OFFSET(refraction, 0xB8);
F3_WATER_OFFSET(flagBC, 0xBC);
F3_WATER_OFFSET(sampleMask, 0xC0);
F3_WATER_OFFSET(tilesX, 0xC4);
F3_WATER_OFFSET(tilesZ, 0xC8);
F3_WATER_OFFSET(texturePatches, 0xCC);
F3_WATER_OFFSET(forceUpdate, 0xD8);
F3_WATER_OFFSET(elapsedTime, 0xDC);
F3_WATER_OFFSET(updateInterval, 0xE0);
F3_WATER_OFFSET(disturbanceMode, 0xE4);
F3_WATER_OFFSET(rippleMinimumLocation, 0xE8);
F3_WATER_OFFSET(rippleMaximumLocation, 0xF0);
F3_WATER_OFFSET(rippleMinimumAmount, 0xF8);
F3_WATER_OFFSET(rippleMaximumAmount, 0xFC);
F3_WATER_OFFSET(rippleMinimumInterval, 0x100);
F3_WATER_OFFSET(rippleMaximumInterval, 0x104);
F3_WATER_OFFSET(rippleNext, 0x108);
F3_WATER_OFFSET(rippleTime, 0x10C);
F3_WATER_OFFSET(lineMinimumOrigin, 0x110);
F3_WATER_OFFSET(lineMaximumOrigin, 0x118);
F3_WATER_OFFSET(lineMinimumDirection, 0x120);
F3_WATER_OFFSET(lineMaximumDirection, 0x128);
F3_WATER_OFFSET(lineMinimumAmount, 0x130);
F3_WATER_OFFSET(lineMaximumAmount, 0x134);
F3_WATER_OFFSET(lineMinimumInterval, 0x138);
F3_WATER_OFFSET(lineMaximumInterval, 0x13C);
F3_WATER_OFFSET(lineNext, 0x140);
F3_WATER_OFFSET(lineTime, 0x144);
F3_WATER_OFFSET(orbitRange, 0x148);
F3_WATER_OFFSET(orbitNext, 0x168);
F3_WATER_OFFSET(orbitTime, 0x16C);
F3_WATER_OFFSET(orbitAngle, 0x170);
F3_WATER_OFFSET(flag174, 0x174);
#undef F3_WATER_OFFSET
#endif
