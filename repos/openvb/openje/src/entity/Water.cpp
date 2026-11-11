#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "Water.hpp"
#include "EntityEnvironment.hpp"
#include "model/WaterTileModelInstance.hpp"
#include "graphics/Water.hpp"

namespace JE
{
	Water::Water()
	{
		model = NULL;
		offsetX = 0;
		offsetZ = 0;
		tileSize = 0;
		gridSize = 0;
		water = NULL;
	}

	Water::~Water()
	{
		delete model; //5AB286, before the embedded configuration strings and DynamicObject.
	}

	void Water::Draw()
	{
		if (!static_cast<Client*>(F3::g_pNetworkInstance->client)->field0x10C && model)
			model->Submit(NULL, 0, 0.05f, 0xFF00FFFF);
	}
	void Water::ClientFrame(DWORD elapsed)
	{
		if (static_cast<Client*>(F3::g_pNetworkInstance->client)->field0x10C != 2 && model) model->UpdateAfterimages(elapsed);
	}
	BYTE Water::AddLight(Gfx_Light* light) { if (model) return model->AddLight(light); return 0; }
	void Water::RemoveLight(Gfx_Light* light) { if (model) model->RemoveLight(light); }
	void Water::ClearLights() { if (model) model->ClearLights(); }
	void Water::FinishLights() { if (model) model->FinishLights(); }

	void Water::AddRipple(float worldX, float worldZ, float amount) //5AB100.
	{
		if (!water) return;
		const D3DXVECTOR3 low = minimum, high = maximum;
		const float width = high.x - low.x;
		const float height = high.z - low.z;
		const float lowX = position.x - width * 0.5f;
		const float lowZ = position.z - height * 0.5f;
		if (!(worldX < lowX) && !(lowX + width <= worldX)
			&& !(worldZ < lowZ) && !(lowZ + height <= worldZ))
		{
			const double ratio = double(tileSize) / gridSize;
			LONG count = static_cast<LONG>(static_cast<__int64>(ratio));
			if (ratio > count) ++count;
			//5AB20E/5AB22A truncate normalized coordinates BEFORE the integer
			//offset additions at5AB221/5AB231. The offsets are not float bits.
			water->AddRipple(static_cast<LONG>(DWORD(static_cast<LONG>(static_cast<__int64>(
				(double(worldX) - lowX) / width * count))) + DWORD(offsetX)),
				static_cast<LONG>(DWORD(static_cast<LONG>(static_cast<__int64>(
				(double(worldZ) - lowZ) / height * count))) + DWORD(offsetZ)), amount);
		}
	}
}
