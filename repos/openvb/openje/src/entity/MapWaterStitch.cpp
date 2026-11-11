#include "Map.hpp"
#include "Water.hpp"
#include "graphics/Water.hpp"
#include "graphics/WaterSampling.hpp"
#include "graphics/G3DTextureImage.hpp"
#include "graphics/SystemInterface.hpp"
#include "resources/ResourceMemoryStream.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cfloat>
#include <new>

namespace
{
	struct WaterModelResource
	{
		WaterModelResource() : model(NULL) {}
		~WaterModelResource() { delete model; }
		void Load(const std::string& name) //529380, fresh5240C0 resource wrapper.
		{
			delete model;
			model = new (std::nothrow) G3D_Model;
			JE::Resources::ResourceMemoryStream* input =
				JE::Resources::OpenResourceMemoryStream(name.c_str(), 0);
			if (input)
			{
				model->Read(input->begin);
				JE::Resources::CloseResourceMemoryStream(input);
			}
		}
		G3D_Model* model;
	};

	struct WaterTextureInput
	{
		G3D_Texture* texture;
		std::string tileName;
		float x, z;
	};

	struct WaterGroup
	{
		WaterGroup() : minimumX(FLT_MAX), minimumZ(FLT_MAX), maximumX(-FLT_MAX),
			maximumZ(-FLT_MAX), minimumY(FLT_MAX), maximumY(-FLT_MAX),
			maximumWidth(LONG(0x80000001)), maximumHeight(LONG(0x80000001)) {}
		float minimumX, minimumZ, maximumX, maximumZ, minimumY, maximumY;
		LONG maximumWidth, maximumHeight;
		std::list<D3DXVECTOR3> positions;
		std::list<WaterTextureInput> textures;
	};

	G3D_Texture* FindTexture(G3D_Model& model, const std::string& name) //4CD040.
	{
		return model.FindTexture(name);
	}

	void AddTextureInput(WaterGroup& group, G3D_Texture* texture,
		const std::string& name, const JE::MapWaterTile& tile)
	{
		JE::Resources::ResourceMemoryStream* stream =
			JE::Resources::OpenResourceMemoryStream(name.c_str(), 0);
		if (stream)
		{
			texture->LoadImage(stream->begin, 0); //Original ignores the result.
			if (group.maximumWidth < LONG(texture->width))
				group.maximumWidth = LONG(texture->width);
			if (group.maximumHeight < LONG(texture->height))
				group.maximumHeight = LONG(texture->height);
			WaterTextureInput input;
			input.texture = texture;
			input.tileName = tile.textureName;
			input.x = tile.position.x;
			input.z = tile.position.z;
			group.textures.push_back(input);
			JE::Resources::CloseResourceMemoryStream(stream);
		}
		else
		{
			JE::LogDebugString("Map::StitchWaterTiles - Missing water texture from resource: %s\n",
				name.c_str());
		}
	}
}

namespace JE
{
	void Map::StitchWaterTiles(float tileSize, float gridSize)
	{
		for (std::list<Gfx_Water*>::iterator i = waterSampleOwners.begin();
			i != waterSampleOwners.end(); ++i)
		{
			RemoveWaterSampleOwners((*i)->registryName);
		}
		waterSampleOwners.clear();
		waterGroups.clear();
		for (std::list<MapWaterTile*>::iterator i = waterTiles.begin(); i != waterTiles.end(); ++i)
		{
			const std::string name = resourceName + "_" + (*i)->configName;
			if (waterGroups.find(name) == waterGroups.end())
			{
				const DWORD index = DWORD(waterGroups.size());
				waterGroups[name] = index;
			}
		}
		if (waterGroups.empty()) return;

		std::vector<WaterGroup> groups(waterGroups.size());
		std::map<std::string,WaterModelResource*> models;
		std::string lastTexture;
		for (std::list<MapWaterTile*>::iterator i = waterTiles.begin(); i != waterTiles.end(); ++i)
		{
			MapWaterTile& tile = **i;
			WaterGroup& group = groups[waterGroups.find(resourceName + "_" + tile.configName)->second];
			if (tile.position.x < group.minimumX) group.minimumX = tile.position.x;
			if (tile.position.z < group.minimumZ) group.minimumZ = tile.position.z;
			const float highX = tile.position.x + tileSize, highZ = tile.position.z + tileSize;
			if (highX > group.maximumX) group.maximumX = highX;
			if (highZ > group.maximumZ) group.maximumZ = highZ;
			if (tile.position.y < group.minimumY) group.minimumY = tile.position.y;
			if (tile.position.y > group.maximumY) group.maximumY = tile.position.y;
			group.positions.push_back(tile.position);

			//5779EC..577A56 tests the same length-4 character for '.', 'b', '3',
			//and 'd', so the original always takes the extension append.
			const std::string modelName = std::string(tile.presetName.c_str()) + ".b3d";
			std::map<std::string,WaterModelResource*>::iterator found = models.find(modelName);
			WaterModelResource* source;
			if (found == models.end())
			{
				source = new WaterModelResource;
				source->Load(modelName);
				models[modelName] = source;
			}
			else source = found->second;
			G3D_Model& model = *source->model;
			std::list<G3D_Node*> pending;
			pending.push_back(model.root);
			while (!pending.empty())
			{
				G3D_Node* node = pending.front();
				pending.pop_front();
				//Original577BF3 counts current children, but577C12 reads root's
				//child array. Preserve that decision rather than repairing traversal.
				for (DWORD child = 0; child < node->children.size(); ++child)
					pending.push_back(model.root->children[child]);
				for (DWORD attribute = 0; attribute < node->attributes.size(); ++attribute)
				{
					G3D_Mesh* mesh = dynamic_cast<G3D_Mesh*>(node->attributes[attribute]);
					if (!mesh) continue;
					tile.mesh.CopyFrom(*mesh);
					tile.converted10C = model.version; //577CCA, distinct from mesh.converted.
					if (!model.version)
					{
						for (DWORD polygon = 0; polygon < mesh->polygons.size(); ++polygon)
						{
							G3D_Texture* texture = FindTexture(*node->model,
								mesh->polygons[polygon]->textures[0]);
							lastTexture = texture->file;
							AddTextureInput(group, texture, lastTexture, tile);
						}
					}
					else
					{
						for (DWORD material = 0; material < mesh->groupCount; ++material)
						{
							lastTexture = mesh->materials[material].textures[0];
							G3D_Texture* texture = FindTexture(*node->model, lastTexture);
							AddTextureInput(group, texture, lastTexture, tile);
						}
					}
				}
			}
		}

		const float inverseGrid = float(1.0 / gridSize);
		const LONG cellsPerTile = LONG((1.0 / gridSize) * tileSize);
		std::map<std::string,DWORD>::iterator name = waterGroups.begin();
		for (DWORD index = 0; index < groups.size(); ++index, ++name)
		{
			WaterGroup& group = groups[index];
			G3D_Transform transform;
			transform.translation[0] = group.minimumX;
			transform.translation[1] = group.maximumY;
			transform.translation[2] = group.minimumZ; //Original local is never applied.
			Gfx_Water* water = new Gfx_Water;
			MapWaterConfiguration configuration;
			std::map<DWORD,MapWaterConfiguration>::iterator config = waterConfigurations.find(name->second);
			if (config != waterConfigurations.end()) configuration = config->second;
			water->registryName = name->first;
			water->sampleWidth = LONG((group.maximumX - group.minimumX) * inverseGrid + 1.0);
			water->sampleHeight = LONG((group.maximumZ - group.minimumZ) * inverseGrid + 1.0);
			water->textureName = lastTexture.c_str();
			water->cellSize = gridSize;
			water->secondaryTextureName = "";
			water->tilesX = cellsPerTile;
			water->tilesZ = cellsPerTile;
			water->disturbanceMode = configuration.value44; //578697: local configuration+44.
			water->SetRippleRange(
				configuration.rippleMinimumLocation,
				configuration.rippleMaximumLocation,
				configuration.rippleMinimumAmount, configuration.rippleMaximumAmount,
				configuration.rippleMinimumInterval, configuration.rippleMaximumInterval);
			water->SetLineRange(
				configuration.lineMinimumOrigin,
				configuration.lineMaximumOrigin,
				configuration.lineMinimumDirection,
				configuration.lineMaximumDirection,
				configuration.lineMinimumAmount, configuration.lineMaximumAmount,
				configuration.lineMinimumInterval, configuration.lineMaximumInterval);
			water->SetOrbitRange(configuration.orbitMinimumRadius, configuration.orbitMaximumRadius,
				configuration.orbitMinimumSpeed, configuration.orbitMaximumSpeed,
				configuration.orbitMinimumAmount, configuration.orbitMaximumAmount,
				configuration.orbitMinimumInterval, configuration.orbitMaximumInterval);
			for (std::list<WaterTextureInput>::iterator texture = group.textures.begin();
				texture != group.textures.end(); ++texture)
			{
				const LONG x = LONG((texture->x - group.minimumX) * double(water->tilesX) / tileSize);
				const LONG z = LONG((texture->z - group.minimumZ) * double(water->tilesZ) / tileSize);
				water->AddTexturePatch(x, z, texture->texture->file, texture->tileName);
			}
			water->PrepareSamples();
			if (water->textureName.find_first_not_of(" \n\t\r") != std::string::npos)
			{
				Resources::ResourceMemoryStream* stream =
					Resources::OpenResourceMemoryStream(water->textureName.c_str(), 0);
				if (stream)
				{
					DWORD handle;
					g_GfxSystem->LoadTextureFromBuffer(&handle, water->textureName, stream->begin,
						UINT(stream->end - stream->begin), D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
						UINT(-1), 3, 5, 0);
					Resources::CloseResourceMemoryStream(stream);
					water->SetTexture(handle, 1);
				}
				else LogDebugString("Map::StitchWaterTiles - Missing water texture from resource: %s\n",
					water->textureName.c_str());
			}
			water->SetRippleRange(D3DXVECTOR2(0, 0), D3DXVECTOR2(1, 1),
				0, float(water->cellSize * 0.1), 0.2f, 2);
			RegisterWaterSampleOwner(water);
			waterSampleOwners.push_back(water);
		}

		for (std::list<MapWaterTile*>::iterator tile = waterTiles.begin(); tile != waterTiles.end(); ++tile)
		{
			const DWORD index = waterGroups.find(resourceName + "_" + (*tile)->configName)->second;
			std::map<DWORD,MapWaterConfiguration>::iterator configuration = waterConfigurations.find(index);
			Water* object = new Water;
			object->Create(**tile, configuration == waterConfigurations.end() ? NULL : &configuration->second,
				LONG(((*tile)->position.x - groups[index].minimumX) * inverseGrid),
				LONG(((*tile)->position.z - groups[index].minimumZ) * inverseGrid),
				tileSize, gridSize, resourceName);
			object->AttachToOctree(GetOctree());
			waterObjects.push_back(object);
			delete *tile;
		}
		waterTiles.clear();
		for (std::map<std::string,WaterModelResource*>::iterator model = models.begin();
			model != models.end(); ++model)
		{
			delete model->second;
		}
	}
}

BYTE G3D_ReadTga(G3D_Texture& texture,const BYTE* data)
{
	return texture.LoadImage(data, 0); //Actual shared4CA2B0 flag0 caller.
}
