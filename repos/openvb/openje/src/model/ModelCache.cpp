#include "ModelCache.hpp"
#include "ModelInfo.hpp"
#include "ModelInstance.hpp"
#include "CommonException.hpp"
#include "graphics/StringFormat.hpp"
#include "configuration/ResourceConfigurationRead.hpp"
#include "network/Types.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <stdexcept>
#include <string.h>

namespace JE
{
	namespace EngineClientInitializationServices
	{
		extern Net::UInt32 g_SelectionEffectArgument; //Existing shared scale word6FF0D8.
	}
	ModelCache* g_pModelCache;
	// Original6FF0DC, initialized from model.ini by6438B0 before the cache.
	//585CE8 copies this actual std::string into the by-value reader argument.
	static std::string modelConfigurationName("model.ini");
	static std::string modelAbbreviationKey("Abbreviation"); //6FF0F8; native6438E0 lifetime.
	static std::string modelVariationCountKey("VariationCount"); //6FF114.
	static std::string textureVariationCountKey("TextureVariationCount"); //6FF130; native643940.
	static std::string appendVariationNameKey("AppendModelVariationName"); //6FF168.
	static std::string appendSexSuffixKey("AppendSexSuffix"); //6FF184.
	static std::string maleSuffixFormat("MaleSuffix%d"); //6FF1A0; native643A00.
	static std::string femaleSuffixFormat("FemaleSuffix%d"); //6FF1BC; native643A30.
	static std::string appendVariationSuffixKey("AppendModelVariationSuffix"); //6FF1D8.
	static std::string variationSuffixFormat("ModelVariationSuffix%d"); //6FF1F4; native643A90.

	ModelCache::ModelCache()
	{
		if (g_pModelCache) throw Common::Exception("ModelCache object already exists.");
		g_pModelCache = this;
		Configuration::ReadResourceConfigurationFile(modelConfigurationName, settings);
		for (slotCount = 1; ; ++slotCount)
		{
			char key[12];
			_itoa(slotCount, key, 10);
			// Original585DA0..585DB4 destroys the temporary key before
			// comparing the captured iterator against the live settings end.
			Configuration::Settings::iterator section = settings.find(key);
			if (section == settings.end()) break;
		}
	}

	ModelCache::~ModelCache()
	{
		for (std::map<std::string,ModelInfo*,Configuration::CaseInsensitiveLess>::iterator entry=entries.begin();
			entry!=entries.end();++entry)
		{
			delete entry->second; //Original deletes the payload before585B70 erase.
			entries.erase(entry);
			//585B79 then increments this erased iterator. Preserve the original
			//source quirk through the real VC7 container, not a safe replacement.
		}
		g_pModelCache = NULL;
	}

	void ModelCache::Initialize() //59C230.
	{
		BeforeInitialize();
	}

	void ModelCache::Cleanup() //5B1510.
	{
		BeforeCleanup();
	}

	void ModelCache::Update() {} //404B70.
	void ModelCache::BeforeInitialize() {} //404B70.
	void ModelCache::BeforeCleanup() {} //404B70.

	std::string ModelCache::GetVariationName(DWORD slot, LONG variation) //583D60.
	{
		char key[12];
		_itoa(slot, key, 10);
		Configuration::Settings::iterator section = settings.find(key);
		Configuration::Values& values = section->second;
		_itoa(variation, key, 10);
		Configuration::Values::iterator value = values.find(key);
		std::string text(value->second);
		return text;
	}

	DWORD ModelCache::GetVariationCount(DWORD slot) //583BF0.
	{
		char sectionName[12];
		_itoa(slot, sectionName, 10);
		Configuration::Settings::iterator section = settings.find(sectionName);
		Configuration::Values::iterator value = section->second.find(modelVariationCountKey);
		std::string text(value->second);
		return DWORD(atol(text.c_str()));
	}

	BYTE ModelCache::AppendsVariationName(DWORD slot) //584390.
	{
		std::string text;
		FormatString(text, "%d", slot);
		Configuration::Settings::iterator section = settings.find(text.c_str());
		Configuration::Values& values = section->second;
		Configuration::Values::iterator value = values.find(appendVariationNameKey);
		if (value == values.end()) return 0;
		text = value->second;
		LONG enabled;
		if (text.empty()) enabled = 0;
		else enabled = atol(text.c_str());
		return enabled != 0;
	}

	BYTE ModelCache::AppendsSexSuffix(DWORD slot) //584560.
	{
		std::string text;
		FormatString(text, "%d", slot);
		Configuration::Settings::iterator section = settings.find(text.c_str());
		Configuration::Values& values = section->second;
		Configuration::Values::iterator value = values.find(appendSexSuffixKey);
		if (value == values.end()) return 0;
		text = value->second;
		LONG enabled;
		if (text.empty()) enabled = 0;
		else enabled = atol(text.c_str());
		return enabled != 0;
	}

	BYTE ModelCache::AppendsVariationSuffix(DWORD slot) //584B10.
	{
		std::string text;
		FormatString(text, "%d", slot);
		Configuration::Settings::iterator section = settings.find(text.c_str());
		Configuration::Values& values = section->second;
		Configuration::Values::iterator value = values.find(appendVariationSuffixKey);
		if (value == values.end()) return 0;
		text = value->second;
		LONG enabled;
		if (text.empty()) enabled = 0;
		else enabled = atol(text.c_str());
		return enabled != 0;
	}

	std::string ModelCache::Property(DWORD slot, const std::string& key, BYTE required) const
	{
		char sectionName[12];
		_itoa(slot, sectionName, 10);
		Configuration::Settings::const_iterator section = settings.find(sectionName);
		if (section == settings.end()) throw std::logic_error("ModelCache: original unchecked missing socket section");
		Configuration::Values::const_iterator value = section->second.find(key);
		if (value == section->second.end())
		{
			if (required) throw std::logic_error("ModelCache: original unchecked missing socket property");
			return std::string();
		}
		return value->second;
	}

	LONG ModelCache::Number(DWORD slot, const char* key, BYTE required) const
	{
		std::string value = Property(slot, key, required);
		return atol(value.c_str());
	}

	ModelInfo* ModelCache::AcquireModelInfo(std::string name, BYTE flag)
	{
		char upperName[260];
		strcpy(upperName, name.c_str());
		_strupr(upperName);
		std::string key(upperName);
		std::map<std::string, ModelInfo*, Configuration::CaseInsensitiveLess>::iterator i = entries.find(key);
		ModelInfo* info;
		if (i != entries.end()) info = i->second;
		else
		{
			float scale;
			info = new ModelInfo(key,
				(memcpy(&scale, &EngineClientInitializationServices::g_SelectionEffectArgument, sizeof(scale)), scale), 1, flag);
			entries.insert(std::map<std::string, ModelInfo*, Configuration::CaseInsensitiveLess>::value_type(key, info));
		}
		return info;
	}

	ModelInstance* ModelCache::AcquireModelInstance(std::string name, BYTE separateTextures,
		const char* texture, BYTE flag)
	{
		if (name.empty()) return NULL;
		ModelInfo* info = AcquireModelInfo(name, flag);
		info->separateTextures = separateTextures;
		return info->CreateInstance(texture);
	}

	void ModelCache::ReleaseModelInstance(ModelInstance* instance)
	{
		ModelInfo* info = instance->info;
		info->ReleaseInstance(instance);
		if (!info->references)
		{
			std::map<std::string, ModelInfo*, Configuration::CaseInsensitiveLess>::iterator i;
			i = entries.find(info->GetName());
			if (i != entries.end()) entries.erase(i);
			delete info;
		}
	}
	std::string ModelCache::GetAbbreviation(DWORD slot) //583A60, required section/key.
	{
		char sectionName[12];
		_itoa(slot, sectionName, 10);
		Configuration::Settings::iterator section = settings.find(sectionName);
		Configuration::Values::iterator value = section->second.find(modelAbbreviationKey);
		std::string text(value->second);
		return text;
	}
	DWORD ModelCache::GetTextureVariationCount(DWORD slot) //583F70, absent/empty value is zero.
	{
		std::string text;
		FormatString(text, "%d", slot);
		Configuration::Settings::iterator section = settings.find(text.c_str());
		Configuration::Values& values = section->second;
		Configuration::Values::iterator value = values.find(textureVariationCountKey);
		if (value == values.end()) return 0;
		text = value->second;
		LONG count;
		if (text.empty()) count = 0;
		else count = atol(text.c_str());
		return DWORD(count);
	}
	std::string ModelCache::GetTextureVariationName(DWORD slot, DWORD index) //584140.
	{
		std::string text;
		FormatString(text, "%d", slot);
		Configuration::Settings::iterator section = settings.find(text.c_str());
		Configuration::Values& values = section->second;
		FormatString(text, "Tex%d", index);
		Configuration::Values::iterator value = values.find(text.c_str());
		if (value == values.end()) text = "";
		else text = value->second;
		return text;
	}
	std::string ModelCache::GetMaleSuffix(DWORD slot, LONG variation) //584730.
	{
		std::string text;
		FormatString(text, "%d", slot);
		Configuration::Settings::iterator section = settings.find(text.c_str());
		Configuration::Values& values = section->second;
		FormatString(text, maleSuffixFormat.c_str(), variation);
		Configuration::Values::iterator value = values.find(text);
		if (value == values.end()) return "";
		return value->second;
	}
	std::string ModelCache::GetFemaleSuffix(DWORD slot, LONG variation) //584920.
	{
		std::string text;
		FormatString(text, "%d", slot);
		Configuration::Settings::iterator section = settings.find(text.c_str());
		Configuration::Values& values = section->second;
		FormatString(text, femaleSuffixFormat.c_str(), variation);
		Configuration::Values::iterator value = values.find(text);
		if (value == values.end()) return "";
		return value->second;
	}
	std::string ModelCache::GetVariationSuffix(DWORD slot, LONG variation) //584CE0.
	{
		std::string text;
		FormatString(text, "%d", slot);
		Configuration::Settings::iterator section = settings.find(text.c_str());
		Configuration::Values& values = section->second;
		FormatString(text, variationSuffixFormat.c_str(), variation);
		Configuration::Values::iterator value = values.find(text);
		if (value == values.end()) return "";
		return value->second;
	}
}
