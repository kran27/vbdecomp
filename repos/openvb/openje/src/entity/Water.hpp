#pragma once

#include "DynamicObject.hpp"
#include "MapWater.hpp"

class Gfx_Water;

namespace JE
{
	class WaterTileModelInstance;

	// Actual spatial Water owner5AB310. Source members use their normal owners;
	// original offsets below identify the recovered payload, not a host overlay.
	class Water : public DynamicObject
	{
	public:
		Water(); //5AB310.
		virtual ~Water(); //5AB250.
		virtual void Draw(); //5AB040; WaterMode0 only.
		virtual void ClientFrame(DWORD elapsed); //5AB070; WaterMode!=2.
		virtual BYTE AddLight(Gfx_Light* light); //5AB0A0.
		virtual void RemoveLight(Gfx_Light* light); //5AB0C0.
		virtual void ClearLights(); //5AB0E0.
		virtual void FinishLights(); //5AB0F0.
		void Create(const MapWaterTile& tile, const MapWaterConfiguration* configuration,
			LONG offsetX, LONG offsetZ, float tileSize, float gridSize,
			const std::string& mapName); //5AB3A0.
		void AddRipple(float worldX, float worldZ, float amount); //5AB100.

		WaterTileModelInstance* model; //+15C, owned.
		LONG offsetX, offsetZ; //+160/+164, integer grid offsets (5AB21B/5AB231).
		float tileSize, gridSize; //+168/+16C.
		Gfx_Water* water; //+170, borrowed registry lookup result.
		MapWaterConfiguration configuration; //+174, copied only for non-NULL input.
	private:
		Water(const Water&);
		Water& operator=(const Water&);
	};
}
