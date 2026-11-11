#include "Water.hpp"
#include "model/WaterTileModelInstance.hpp"
#include "graphics/G3DWaterTile.hpp"

namespace JE
{
	void Water::Create(const MapWaterTile& input, const MapWaterConfiguration* settings,
		LONG first, LONG row, float size, float spacing, const std::string& mapName) //5AB3A0.
	{
		if (settings) configuration = *settings; //571110; no default scalar values on NULL.
		G3D_Model source;
		G3D_Node* node = new G3D_Node;
		G3D_WaterTile* tile = new G3D_WaterTile;
		tile->CopyFrom(input.mesh);
		tile->sourceVersion = input.converted10C;
		if (input.converted10C)
		{
			if (tile->groupCount > 0)
			{
				G3D_MeshMaterial* material = tile->materials;
				material->textureCount = 2;
				material->textures[1] = input.textureName;
			}
		}
		else if (static_cast<LONG>(tile->polygons.size()) > 0)
		{
			G3D_Polygon* polygon = tile->polygons[0];
			polygon->textureCount = 2;
			polygon->textures[1] = input.textureName;
		}
		tile->offsetX = first;
		tile->offsetZ = row;
		tile->columns = static_cast<LONG>(static_cast<__int64>(double(size) / spacing + 1.0f));
		tile->rows = tile->columns;
		std::string name = mapName;
		name += "_";
		name += input.configName;
		node->name = name;
		node->AddAttribute(tile); //5AB594; registration precedes owner publication.
		source.root->AddChild(node);
		WaterTileModelInstance* instance = new WaterTileModelInstance(10.0f);
		instance->SetModel(source, input, settings, size);
		D3DXVECTOR3 low, high;
		instance->GetBounds(low, high);
		const D3DXVECTOR3 center(float(double(size) * 0.5 + input.position.x), input.position.y,
			float(double(size) * 0.5 + input.position.z));
		instance->SetPosition(center.x, center.y, center.z);
		model = instance;
		ChangePosition(center);
		minimum = low;
		maximum = high;
		offsetX = first;
		offsetZ = row;
		tileSize = size;
		gridSize = spacing;
		water = instance->water;
	}
}
