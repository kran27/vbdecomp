#include "WaterTileModelInstance.hpp"
#include "entity/MapWater.hpp"
#include "graphics/G3DModel.hpp"
#include "graphics/ModelData.hpp"
#include "graphics/ModelInstance.hpp"
#include "graphics/ModelNode.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/WaterTile.hpp"
#include <list>
#include <stdexcept>

namespace JE
{
	WaterTileModelInstance::WaterTileModelInstance(float value)
		: ModelInstance(std::string(""), 0, 0), conversionScale(value), dataHandle(0), tileSize(0), water(NULL) {}

	WaterTileModelInstance::~WaterTileModelInstance()
	{
		if (instances[0])
		{
			delete instances[0];
			instances[0] = NULL;
		}
		if (dataHandle)
		{
			g_GfxSystem->ReleaseModelData(&dataHandle);
			dataHandle = 0;
		}
	}

	void WaterTileModelInstance::GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high)
	{
		low.x = 0;
		low.y = 0;
		low.z = 0;
		high.x = tileSize;
		high.y = 0;
		high.z = tileSize;
	}

	void WaterTileModelInstance::ReleaseData() //563BB0.
	{
		if (g_GfxSystem && dataHandle)
		{
			g_GfxSystem->ReleaseModelData(&dataHandle);
			dataHandle = 0;
		}
	}

	Gfx_ModelInstance* WaterTileModelInstance::BuildModel(G3D_Model& source,
		const MapWaterTile& input, const MapWaterConfiguration* configuration) //565BB0.
	{
		//529D80 constructs the actual model data and publishes its registration,
		//then the graphics owner creates the instance through its normal factory.
		dataHandle = Gfx_LoadModelData(&source, 1, 1, 0, conversionScale, 338, NULL, 1);
		Gfx_ModelInstance* result = g_GfxSystem->CreateModelInstance(dataHandle);
		std::list<Gfx_ModelNode*> pending;
		pending.push_back(result->root);
		while (!pending.empty())
		{
			Gfx_ModelNode* node = pending.front();
			pending.pop_front();
			const std::vector<Gfx_NodeAttribute*> attributes(node->GetAttributes()); //565AA0, copied vector.
			for (size_t i = 0; i < attributes.size(); ++i)
				if (Gfx_WaterTile* tile = dynamic_cast<Gfx_WaterTile*>(attributes[i]))
				{
					water = tile->GetWater();
					offsetX = tile->offsetX;
					offsetZ = tile->offsetZ;
					columns = tile->columns;
					rows = tile->rows;
					tile->flag224 = configuration->flag3C;
					tile->flag225 = configuration->flag3D;
					tile->flag226 = configuration->flag3E;
					tile->value23C = configuration->value40;
					tile->Rebuild();
					tile->SetTextureUV(input.u, input.v, input.width, input.height);
					break; //Original finds only the first WaterTile attribute per node.
				}
			const std::vector<Gfx_ModelNode*> children(node->GetChildren()); //565AD0, after tile callbacks.
			for (size_t i = 0; i < children.size(); ++i) pending.push_back(children[i]);
		}
		return result;
	}

	void WaterTileModelInstance::SetModel(G3D_Model& source, const MapWaterTile& tile,
		const MapWaterConfiguration* configuration, float size) //565F50.
	{
		tileSize = size;
		if (instances[0])
		{
			Gfx_ModelData526620* data = instances[0]->data;
			for (DWORD i = 0; i < data->extraNameCount; ++i) instances[0]->EnableMaterial(data->extraNames[i], 1);
			delete instances[0];
			instances[0] = NULL;
			ReleaseData();
			variations[0] = DWORD(-1);
		}
		instances[0] = BuildModel(source, tile, configuration);
		variations[0] = 0;
	}
}
