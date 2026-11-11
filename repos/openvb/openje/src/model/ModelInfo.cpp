#include "ModelInfo.hpp"
#include "ModelCache.hpp"
#include "ModelInstance.hpp"
#include "graphics/ModelData.hpp"
#include "graphics/SystemInterface.hpp"
#include "resources/ResourceSystem.hpp"
#include <stdlib.h>
#include <string.h>

namespace JE
{
	static std::string modelFileExtension(".B3D"); //6FEF74, native6434C0.
	static std::string textureFileExtension(".DDS"); //6FEF90, native6434F0.
	DWORD ModelInfo::GetSocketModelCount(DWORD slot)
	{
		return DWORD(models[slot].size());
	}
	DWORD ModelInfo::GetSocketTextureCount(DWORD slot, const char* variation) //566D60.
	{
		if (variation && (g_pModelCache->AppendsVariationName(slot) ||
			g_pModelCache->AppendsSexSuffix(slot) || g_pModelCache->AppendsVariationSuffix(slot)) &&
			g_pModelCache->GetVariationCount(slot) > 0)
		{
			std::string key(variation);
			return DWORD(variationTextures[slot][key].size());
		}
		return DWORD(textures[slot].size());
	}
	std::string ModelInfo::GetSocketTextureName(DWORD slot, DWORD index, const char* variation) //566E90.
	{
		if ((g_pModelCache->AppendsVariationName(slot) || g_pModelCache->AppendsSexSuffix(slot) ||
			g_pModelCache->AppendsVariationSuffix(slot)) && variation &&
			g_pModelCache->GetVariationCount(slot) > 0)
			return variationTextures[slot][variation][index];
		return textures[slot][index];
	}
	std::string ModelInfo::GetSocketModelName(DWORD slot, DWORD variation) //5647E0.
	{
		ModelVariation candidate = models[slot][variation];
		return candidate.name;
	}
	BYTE ModelInfo::FindSocketModel(DWORD slot, const std::string& requestedName, DWORD* variation)
	{
		const DWORD count = DWORD(models[slot].size());
		for (DWORD i = 0; i < count; ++i)
		{
			{
				ModelVariation candidate = models[slot][i];
				if (!strstr(candidate.name.c_str(), requestedName.c_str())) continue;
			}
			*variation = i;
			return 1;
		}
		return 0;
	}
	ModelInfo::ModelInfo(std::string inputName, float inputScale, BYTE separate, BYTE flag)
	{
		const DWORD count = g_pModelCache->slotCount;
		models = new std::vector<ModelVariation>[count];
		textures = new std::vector<std::string>[count];
		variationTextures = new VariationTextures[count];
		references = 0;
		char upperName[260];
		strcpy(upperName, inputName.c_str());
		_strupr(upperName);
		name = upperName;
		scale = inputScale;
		separateTextures = separate;
		animationFlag = flag;
		char resourceName[260];
		for (DWORD slot = 0; slot < count; ++slot)
		{
			if (!slot)
			{
				std::string modelBase(inputName);
				std::string candidate = modelBase + modelFileExtension;
				ModelVariation model;
				if (Resources::g_pResourceSystem->Contains(candidate.c_str()) == 1)
				{
					strcpy(resourceName, candidate.c_str());
					_strupr(resourceName);
					model.name = resourceName;
				}
				models[slot].push_back(model);
			}
			else
			{
				std::string modelBase = inputName + "_";
				std::string modelAbbreviation = g_pModelCache->GetAbbreviation(slot);
				modelBase += modelAbbreviation;
				DWORD variations = g_pModelCache->GetVariationCount(slot);
				for (DWORD variation = 0; variation < variations; ++variation)
				{
					char number[12]; _itoa(variation, number, 10); //Native redundant conversion56743B.
					std::string variationName = g_pModelCache->GetVariationName(slot, variation);
					std::string candidate = modelBase + variationName + modelFileExtension;
					ModelVariation model;
					if (Resources::g_pResourceSystem->Contains(candidate.c_str()) == 1)
					{
						strcpy(resourceName, candidate.c_str());
						_strupr(resourceName);
						model.name = resourceName;
					}
					models[slot].push_back(model);
				}
			}
			std::string abbreviation = g_pModelCache->GetAbbreviation(slot);
			std::string textureBase(abbreviation);
			BYTE appendName = g_pModelCache->AppendsVariationName(slot);
			BYTE appendSuffix = g_pModelCache->AppendsVariationSuffix(slot);
			BYTE appendSex = g_pModelCache->AppendsSexSuffix(slot);
			LONG textureVariations = 0;
			if (appendName || appendSuffix || appendSex)
				textureVariations = LONG(g_pModelCache->GetVariationCount(slot));
			LONG groups = textureVariations > 0 ? textureVariations : 1;
			for (LONG variation = 0; variation < groups; ++variation)
			{
				std::string prefix(textureBase), variationName;
				if (appendName || appendSuffix || appendSex)
				{
					if (textureVariations > 0) variationName = g_pModelCache->GetVariationName(slot, variation);
					if (appendName) prefix += variationName;
					if (appendSuffix)
					{
						prefix += g_pModelCache->GetVariationSuffix(slot, variation);
					}
					if (appendSex)
					{
						std::string suffix;
						if (inputName.c_str()[3] == 'f' || inputName.c_str()[3] == 'F')
							suffix = g_pModelCache->GetFemaleSuffix(slot, variation);
						else suffix = g_pModelCache->GetMaleSuffix(slot, variation);
						prefix += suffix;
					}
				}
				DWORD textureCount = g_pModelCache->GetTextureVariationCount(slot);
				for (DWORD texture = 0; texture < textureCount; ++texture)
				{
					std::string base = prefix + g_pModelCache->GetTextureVariationName(slot, texture) + "_LG";
					std::string candidate = base + textureFileExtension;
					std::string selected;
					if (Resources::g_pResourceSystem->Contains(candidate.c_str()) == 1)
					{
						strcpy(resourceName, candidate.c_str());
						_strupr(resourceName);
						selected = resourceName;
					}
					if ((appendName || appendSuffix || appendSex) && textureVariations > 0)
						variationTextures[slot][variationName].push_back(selected);
					else textures[slot].push_back(selected);
				}
			}
		}
	}

	ModelInfo::~ModelInfo()
	{
		const DWORD count = g_pModelCache->slotCount;
		Gfx_System_Interface* graphics = g_GfxSystem;
		for (DWORD slot = 0; slot < count; ++slot)
		{
			for (std::vector<ModelVariation>::iterator model = models[slot].begin();
				model != models[slot].end(); ++model)
			{
				if (graphics && model->handle) { graphics->ReleaseModelData(&model->handle); model->handle = 0; }
			}
			models[slot].clear();
		}
		delete[] models;
		delete[] variationTextures;
		delete[] textures;
	}

	Gfx_ModelInstance* ModelInfo::Acquire(DWORD slot, DWORD variation)
	{
		Gfx_ModelInstance* instance = NULL;
		ModelVariation model = models[slot][variation];
		if (!model.name.empty())
		{
			if (!model.handle)
			{
				model.handle = Gfx_LoadModelData(model.name, 1, separateTextures == 0, 1, scale, 338);
				models[slot][variation].handle = model.handle;
			}
			instance = g_GfxSystem->CreateModelInstance(model.handle);
			++models[slot][variation].references;
		}
		return instance;
	}

	void ModelInfo::Release(DWORD slot, DWORD variation)
	{
		--models[slot][variation].references;
		if (!models[slot][variation].references && g_GfxSystem && models[slot][variation].handle)
		{
			g_GfxSystem->ReleaseModelData(&models[slot][variation].handle);
			models[slot][variation].handle = 0;
		}
	}

	ModelInstance* ModelInfo::CreateInstance(const char* texture)
	{
		ModelInstance* instance = new ModelInstance(name, 0, animationFlag);
		instance->info = this;
		instance->SetSocket(0, 0, texture);
		++references;
		return instance;
	}
	void ModelInfo::ReleaseInstance(ModelInstance* instance)
	{
		if (instance) delete instance;
		--references;
	}
	std::string ModelInfo::GetName() const
	{
		return name;
	}
}
