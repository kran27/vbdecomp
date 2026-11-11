#pragma once

#include "graphics/G3DModel.hpp"
#include <d3dx8.h>
#include <string>
#include <cstddef>

namespace JE
{
	// The value stored in Map's original+E0 table. Offsets identify the original
	// fields; this is an ordinary C++ object, not an overlay of the old layout.
	struct MapWaterConfiguration
	{
		MapWaterConfiguration() {} // Strings construct; scalar payload is read by575A40.
		std::string presetName; //+00.
		std::string name; //+1C, synthesized Water%d.
		DWORD id; //+38, the table size before insertion.
		BYTE flag3C, flag3D, flag3E, flag3F;
		float value40;
		DWORD value44, value48;
		float value4C, value50;
		DWORD value54;
		D3DXVECTOR2 rippleMinimumLocation; //+58, SetRippleRange.minimumLocation.
		D3DXVECTOR2 rippleMaximumLocation; //+60, SetRippleRange.maximumLocation.
		float rippleMinimumAmount; //+68, SetRippleRange.minimumAmount.
		float rippleMaximumAmount; //+6C, SetRippleRange.maximumAmount.
		float rippleMinimumInterval; //+70, SetRippleRange.minimumInterval.
		float rippleMaximumInterval; //+74, SetRippleRange.maximumInterval.
		D3DXVECTOR2 lineMinimumOrigin; //+78, SetLineRange.minimumOrigin.
		D3DXVECTOR2 lineMaximumOrigin; //+80, SetLineRange.maximumOrigin.
		D3DXVECTOR2 lineMinimumDirection; //+88, SetLineRange.minimumDirection.
		D3DXVECTOR2 lineMaximumDirection; //+90, SetLineRange.maximumDirection.
		float lineMinimumAmount; //+98, SetLineRange.minimumAmount.
		float lineMaximumAmount; //+9C, SetLineRange.maximumAmount.
		float lineMinimumInterval; //+A0, SetLineRange.minimumInterval.
		float lineMaximumInterval; //+A4, SetLineRange.maximumInterval.
		float orbitMinimumRadius; //+A8, SetOrbitRange.minimumRadius.
		float orbitMaximumRadius; //+AC, SetOrbitRange.maximumRadius.
		float orbitMinimumSpeed; //+B0, SetOrbitRange.minimumSpeed.
		float orbitMaximumSpeed; //+B4, SetOrbitRange.maximumSpeed.
		float orbitMinimumAmount; //+B8, SetOrbitRange.minimumAmount.
		float orbitMaximumAmount; //+BC, SetOrbitRange.maximumAmount.
		float orbitMinimumInterval; //+C0, SetOrbitRange.minimumInterval.
		float orbitMaximumInterval; //+C4, SetOrbitRange.maximumInterval.
		LONG tileCount; //+C8; original signed loop bound.
	};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char MapWaterConfigurationSizeCC[(sizeof(MapWaterConfiguration) == 0xCC) ? 1 : -1];
	typedef char MapWaterConfigurationVector2Size[(sizeof(D3DXVECTOR2) == 8) ? 1 : -1];
	typedef char MapWaterConfiguration_rippleMinimumLocation[(offsetof(MapWaterConfiguration, rippleMinimumLocation) == 0x58) ? 1 : -1];
	typedef char MapWaterConfiguration_rippleMinimumLocation_x[(offsetof(MapWaterConfiguration, rippleMinimumLocation) + offsetof(D3DXVECTOR2, x) == 0x58) ? 1 : -1];
	typedef char MapWaterConfiguration_rippleMinimumLocation_y[(offsetof(MapWaterConfiguration, rippleMinimumLocation) + offsetof(D3DXVECTOR2, y) == 0x5c) ? 1 : -1];
	typedef char MapWaterConfiguration_rippleMaximumLocation[(offsetof(MapWaterConfiguration, rippleMaximumLocation) == 0x60) ? 1 : -1];
	typedef char MapWaterConfiguration_rippleMaximumLocation_x[(offsetof(MapWaterConfiguration, rippleMaximumLocation) + offsetof(D3DXVECTOR2, x) == 0x60) ? 1 : -1];
	typedef char MapWaterConfiguration_rippleMaximumLocation_y[(offsetof(MapWaterConfiguration, rippleMaximumLocation) + offsetof(D3DXVECTOR2, y) == 0x64) ? 1 : -1];
	typedef char MapWaterConfiguration_rippleMinimumAmount[(offsetof(MapWaterConfiguration, rippleMinimumAmount) == 0x68) ? 1 : -1];
	typedef char MapWaterConfiguration_rippleMaximumAmount[(offsetof(MapWaterConfiguration, rippleMaximumAmount) == 0x6c) ? 1 : -1];
	typedef char MapWaterConfiguration_rippleMinimumInterval[(offsetof(MapWaterConfiguration, rippleMinimumInterval) == 0x70) ? 1 : -1];
	typedef char MapWaterConfiguration_rippleMaximumInterval[(offsetof(MapWaterConfiguration, rippleMaximumInterval) == 0x74) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMinimumOrigin[(offsetof(MapWaterConfiguration, lineMinimumOrigin) == 0x78) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMinimumOrigin_x[(offsetof(MapWaterConfiguration, lineMinimumOrigin) + offsetof(D3DXVECTOR2, x) == 0x78) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMinimumOrigin_y[(offsetof(MapWaterConfiguration, lineMinimumOrigin) + offsetof(D3DXVECTOR2, y) == 0x7c) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMaximumOrigin[(offsetof(MapWaterConfiguration, lineMaximumOrigin) == 0x80) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMaximumOrigin_x[(offsetof(MapWaterConfiguration, lineMaximumOrigin) + offsetof(D3DXVECTOR2, x) == 0x80) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMaximumOrigin_y[(offsetof(MapWaterConfiguration, lineMaximumOrigin) + offsetof(D3DXVECTOR2, y) == 0x84) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMinimumDirection[(offsetof(MapWaterConfiguration, lineMinimumDirection) == 0x88) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMinimumDirection_x[(offsetof(MapWaterConfiguration, lineMinimumDirection) + offsetof(D3DXVECTOR2, x) == 0x88) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMinimumDirection_y[(offsetof(MapWaterConfiguration, lineMinimumDirection) + offsetof(D3DXVECTOR2, y) == 0x8c) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMaximumDirection[(offsetof(MapWaterConfiguration, lineMaximumDirection) == 0x90) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMaximumDirection_x[(offsetof(MapWaterConfiguration, lineMaximumDirection) + offsetof(D3DXVECTOR2, x) == 0x90) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMaximumDirection_y[(offsetof(MapWaterConfiguration, lineMaximumDirection) + offsetof(D3DXVECTOR2, y) == 0x94) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMinimumAmount[(offsetof(MapWaterConfiguration, lineMinimumAmount) == 0x98) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMaximumAmount[(offsetof(MapWaterConfiguration, lineMaximumAmount) == 0x9c) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMinimumInterval[(offsetof(MapWaterConfiguration, lineMinimumInterval) == 0xa0) ? 1 : -1];
	typedef char MapWaterConfiguration_lineMaximumInterval[(offsetof(MapWaterConfiguration, lineMaximumInterval) == 0xa4) ? 1 : -1];
	typedef char MapWaterConfiguration_orbitMinimumRadius[(offsetof(MapWaterConfiguration, orbitMinimumRadius) == 0xa8) ? 1 : -1];
	typedef char MapWaterConfiguration_orbitMaximumRadius[(offsetof(MapWaterConfiguration, orbitMaximumRadius) == 0xac) ? 1 : -1];
	typedef char MapWaterConfiguration_orbitMinimumSpeed[(offsetof(MapWaterConfiguration, orbitMinimumSpeed) == 0xb0) ? 1 : -1];
	typedef char MapWaterConfiguration_orbitMaximumSpeed[(offsetof(MapWaterConfiguration, orbitMaximumSpeed) == 0xb4) ? 1 : -1];
	typedef char MapWaterConfiguration_orbitMinimumAmount[(offsetof(MapWaterConfiguration, orbitMinimumAmount) == 0xb8) ? 1 : -1];
	typedef char MapWaterConfiguration_orbitMaximumAmount[(offsetof(MapWaterConfiguration, orbitMaximumAmount) == 0xbc) ? 1 : -1];
	typedef char MapWaterConfiguration_orbitMinimumInterval[(offsetof(MapWaterConfiguration, orbitMinimumInterval) == 0xc0) ? 1 : -1];
	typedef char MapWaterConfiguration_orbitMaximumInterval[(offsetof(MapWaterConfiguration, orbitMaximumInterval) == 0xc4) ? 1 : -1];
	typedef char MapWaterConfigurationTileCountC8[(offsetof(MapWaterConfiguration, tileCount) == 0xC8) ? 1 : -1];
#endif

	// Original272-byte tile record56EB70, owned by Map's+D4 list until stitching
	// or destruction. Reuses the existing actual G3D_Mesh geometry owner.
	struct MapWaterTile
	{
		MapWaterTile();
		D3DXVECTOR3 position; //+00.
		std::string configName; //+0C.
		std::string presetName; //+28.
		std::string textureName; //+44.
		float u, v, width, height; //+60..6C; constructor leaves these unwritten.
		G3D_Mesh mesh; //+70, ctor4D7A20/dtor4D9130 already recovered.
		DWORD converted10C; //+10C, constructor/reader leave unwritten; mesh producer owns it.
	private:
		MapWaterTile(const MapWaterTile&);
		MapWaterTile& operator=(const MapWaterTile&);
	};
}
