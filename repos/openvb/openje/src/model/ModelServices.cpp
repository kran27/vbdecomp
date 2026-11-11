#include "ModelServices.hpp"
#include "configuration/ResourceConfigurationRead.hpp"
#include "resources/ResourceSystem.hpp"
#include "resources/ResourceFormats.hpp"
#include "graphics/SystemInterface.hpp"
#include <d3dx8.h>
#include <string.h>

namespace JE { namespace ModelServices
{
	BYTE ReadSettings(const std::string& name, Configuration::Settings& settings)
	{
		return Configuration::ReadResourceConfigurationFile(name, settings);
	}
	BYTE ResourceExists(const char* name)
	{
		std::string copy(name);
		DWORD type = Resources::ExtractFileType(copy);
		Resources::ResourceSystem* system = Resources::g_pResourceSystem;
		system->lock.Lock();
		Resources::ResourceDescriptor* descriptor = system->temporaryFileIndex.Find(copy.c_str(), type);
		if (!descriptor) descriptor = system->primary.Find(copy.c_str(), type);
		system->lock.Unlock();
		return descriptor != NULL;
	}
	std::string UpperName(const std::string& name)
	{
		char buffer[260];
		strcpy(buffer, name.c_str());
		_strupr(buffer);
		return std::string(buffer);
	}
	BYTE LoadSocketTexture(DWORD* output, const std::string& name, HRESULT* result)
	{
		Resources::ResourceMemoryStream* stream = Resources::OpenResourceMemoryStream(name.c_str(), 0);
		if (!stream) return 0;
		*result = g_GfxSystem->LoadTextureFromBuffer(output, name, stream->begin,
			static_cast<UINT>(stream->end - stream->begin), D3DFMT_A8R8G8B8,
			D3DPOOL_SYSTEMMEM, UINT(-1), D3DX_FILTER_NONE, D3DX_FILTER_TRIANGLE, 0);
		Resources::CloseResourceMemoryStream(stream);
		return 1;
	}
}}
