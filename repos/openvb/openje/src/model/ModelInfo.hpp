#pragma once

#include "ModelServices.hpp"
#include <vector>

class Gfx_ModelInstance;

namespace JE
{
	class ModelInstance;
	struct ModelVariation
	{
		ModelVariation() : handle(0), references(0) {}
		DWORD handle;
		std::string name;
		DWORD references;
	};
	class ModelInfo
	{
	public:
		ModelInfo(std::string name, float scale, BYTE separateTextures, BYTE flag); // 0x567010.
		~ModelInfo(); // 0x567F50.
		Gfx_ModelInstance* Acquire(DWORD slot, DWORD variation); // 0x5646B0.
		void Release(DWORD slot, DWORD variation); // 0x563C40.
		ModelInstance* CreateInstance(const char* texture); // 0x565760.
		void ReleaseInstance(ModelInstance* instance); //5638E0; delete, then decrement.
		std::string GetName() const; //5839F0; owned name value copy.
		DWORD GetSocketModelCount(DWORD slot); //563A00.
		std::string GetSocketModelName(DWORD slot, DWORD variation); //5647E0, unchecked value-copy query.
		DWORD GetSocketTextureCount(DWORD slot, const char* variation); //566D60.
		std::string GetSocketTextureName(DWORD slot, DWORD index, const char* variation); //566E90.
		BYTE FindSocketModel(DWORD slot, const std::string& name, DWORD* variation); // 0x564B90.
		std::vector<ModelVariation>* models; // Owned slotCount-element array.
		DWORD references;
		std::string name;
		float scale;
		BYTE separateTextures, animationFlag;
		std::vector<std::string>* textures; // Owned slotCount-element array.
		typedef std::map<std::string, std::vector<std::string>, Configuration::CaseInsensitiveLess> VariationTextures;
		VariationTextures* variationTextures; // Owned slotCount-element array.
	private:
		ModelInfo(const ModelInfo&);
		ModelInfo& operator=(const ModelInfo&);
	};
}
