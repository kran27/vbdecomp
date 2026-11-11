#include "GraphicsShadowMode.hpp"
#include "SystemInterface.hpp"
#include "OctreeRender.hpp"
#include "resources/ResourceMemoryStream.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <d3dx8.h>
#include <dxerr8.h>
#include <cstdlib>

namespace JE
{
	void InitializeProjectedShadowTexture(Gfx_System_Interface& system)
	{
		Resources::ResourceMemoryStream* stream = Resources::OpenResourceMemoryStream("ProjectedShadow.tga", 0);
		if (!stream)
		{
			LogDebugString("Graphics::Initialize - Cannot load \"ProjectedShadow.tga\" for projected shadow\n");
			return;
		}
		DWORD handle; //Actual TextureManager writes output on cache hit and miss, including decode failure.
		HRESULT result = system.LoadTextureFromBuffer(&handle, "ProjectedShadow.tga", stream->begin,
			UINT(stream->end - stream->begin), D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT,
			D3DX_FILTER_NONE, D3DX_FILTER_BOX, 0);
		if (FAILED(result))
			LogDebugString("Graphics::Initialize - Failed to load \"ProjectedShadow.tga\" from buffer (%x %s)\n",
				result, DXGetErrorDescription8A(result));
		Resources::CloseResourceMemoryStream(stream);
		system.SetProjectedShadowTexture(handle);
	}

	void ReadShadowConfiguration(DWORD& mode, const Configuration::Settings& settings)
	{
		Configuration::Settings::const_iterator section = settings.find("Graphics");
		if (section == settings.end()) return;
		Configuration::Values::const_iterator value = section->second.find("shadow type");
		if (value == section->second.end()) return;
		DWORD parsed = static_cast<DWORD>(std::atol(value->second.c_str()));
		mode = parsed < 3 ? parsed : 1;
	}
	void SetGraphicsShadowMode(DWORD& mode, Octree2Gfx* floor,
		Gfx_System_Interface& system, DWORD requested)
	{
		if (requested != 0 && requested != 2)
		{
			system.SetRendererFlag56(0);
			system.EnableProjectedShadow(1);
			if (floor) floor->floorEnabled = 1;
		}
		else
		{
			system.SetRendererFlag56(requested == 2);
			system.EnableProjectedShadow(0);
			if (floor) floor->floorEnabled = 0;
		}
		mode = requested;
	}
	DWORD CycleGraphicsShadowMode(DWORD& mode, Octree2Gfx* floor, Gfx_System_Interface& system)
	{
		mode = (mode + 1u) % 3u;
		SetGraphicsShadowMode(mode, floor, system, mode);
		return mode;
	}
}
