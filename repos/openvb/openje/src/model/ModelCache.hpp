#pragma once

#include "ModelServices.hpp"
#include "entity/IComponent.hpp"

namespace JE
{
	class ModelInfo;
	class ModelInstance;
	class ModelCache : public IComponent
	{
	public:
		ModelCache(); // 0x585C00; publishes before resource-backed model.ini read.
		virtual ~ModelCache(); // 0x585B10.
		virtual void Initialize(); //59C230, actual component+04 delegates to+10.
		virtual void Cleanup(); //5B1510, actual component+08 delegates to+14.
		virtual void Update(); //404B70, actual component+0C no-op.
		ModelInfo* AcquireModelInfo(std::string name, BYTE flag); // 0x5857F0.
		ModelInstance* AcquireModelInstance(std::string name, BYTE separateTextures,
			const char* texture, BYTE flag); // 0x585A50.
		void ReleaseModelInstance(ModelInstance* instance); // 0x5853E0/5638E0.
		std::string GetAbbreviation(DWORD slot); //583A60; unchecked required model-socket key.
		std::string GetVariationName(DWORD slot, LONG variation); //583D60, unchecked numbered value.
		std::string GetMaleSuffix(DWORD slot, LONG variation); //584730; optional numbered suffix.
		std::string GetFemaleSuffix(DWORD slot, LONG variation); //584920; optional numbered suffix.
		std::string GetVariationSuffix(DWORD slot, LONG variation); //584CE0; optional numbered suffix.
		std::string GetTextureVariationName(DWORD slot, DWORD index); //584140; optional Tex%d value.
		DWORD GetTextureVariationCount(DWORD slot); //583F70; optional inner property, required outer section.
		DWORD GetVariationCount(DWORD slot); //583BF0, unchecked required configuration value.
		BYTE AppendsVariationName(DWORD slot); //584390, absent optional key is false.
		BYTE AppendsVariationSuffix(DWORD slot); //584B10.
		BYTE AppendsSexSuffix(DWORD slot); //584560.
		std::string Property(DWORD slot, const std::string& key, BYTE required) const;
		LONG Number(DWORD slot, const char* key, BYTE required) const;
		Configuration::Settings settings;
		std::map<std::string, ModelInfo*, Configuration::CaseInsensitiveLess> entries;
		DWORD slotCount; // Original+1C, first missing numbered section starting1.
	protected:
		virtual void BeforeInitialize(); //404B70, original+10 no-op hook.
		virtual void BeforeCleanup(); //404B70, original+14 no-op hook.
	private:
		ModelCache(const ModelCache&);
		ModelCache& operator=(const ModelCache&);
	};
	extern ModelCache* g_pModelCache; // Original70C528.
}
