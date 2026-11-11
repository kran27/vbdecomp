#pragma once

#include "ModelInstance.hpp"

class G3D_Model;
class Gfx_Water;

namespace JE
{
	struct MapWaterTile;
	struct MapWaterConfiguration;

	class WaterTileModelInstance : public ModelInstance
	{
	public:
		explicit WaterTileModelInstance(float conversionScale); //565540.
		virtual ~WaterTileModelInstance(); //564620; deletes slot0 before releasing its handle.
		virtual void GetBounds(D3DXVECTOR3& minimum, D3DXVECTOR3& maximum); //5639A0.
		void SetModel(G3D_Model& source, const MapWaterTile& tile,
			const MapWaterConfiguration* configuration, float tileSize); //565F50.
		float conversionScale; //+54.
		DWORD dataHandle; //+58, owned registration.
		float tileSize; //+5C.
		Gfx_Water* water; //+60, borrowed name lookup result.
		LONG offsetX, offsetZ, columns, rows; //+64..70, copied from tile; ctor-unwritten.
	private:
		Gfx_ModelInstance* BuildModel(G3D_Model& source, const MapWaterTile& tile,
			const MapWaterConfiguration* configuration); //565BB0.
		void ReleaseData(); //563BB0, guards graphics singleton then handle.
		WaterTileModelInstance(const WaterTileModelInstance&);
		WaterTileModelInstance& operator=(const WaterTileModelInstance&);
	};
}
