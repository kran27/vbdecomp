#include "RenderStyle.hpp"
#include "RSExpand.hpp"
#include "Light.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include <cstring>

Gfx_RenderStyleController::Gfx_RenderStyleController(Gfx_RenderStyle* const* initial)
{
	activeIndex = 0;
	styles[0] = initial[0];
	styles[1] = initial[1];
	styles[2] = initial[2];
	styles[3] = initial[3];
	styles[4] = initial[4];
}

HRESULT Gfx_RenderStyleController::SetMaterial(Gfx_Material* material, BYTE techniqueFlag,
	BYTE fogOfWar, BYTE heightFogEnabled, DWORD heightFog)
{
	activeIndex = 0;
	if (fogOfWar)
	{
		if (styles[3])
		{
			activeIndex = 3;
			if (heightFogEnabled && heightFog && styles[4])
			{
				activeIndex = 4;
			}
		}
	}
	else if (heightFogEnabled && heightFog && styles[1])
	{
		activeIndex = 1;
	}
	if (this == &g_ShadowRenderStyle)
	{
		if (techniqueFlag)
			styles[activeIndex]->SelectTechnique(1);
		else
			styles[activeIndex]->SelectTechnique(0);
	}
	return styles[activeIndex]->SetMaterial(material, heightFog);
}

void Gfx_ResetLights()
{
	for (DWORD index = 0; index < g_GfxSystem->ActiveLightLimit(); ++index)
	{
		g_D3D.device->LightEnable(index, FALSE);
	}
}

void Gfx_ApplyLights(const std::list<Gfx_Light*>* lights, const D3DXVECTOR4* extra,
	DWORD viewport)
{
	if (lights)
	{
		DWORD available = g_GfxSystem->ActiveLightLimit();
		const BYTE automap = g_GfxSystem->AutomapReady();
		if (extra)
		{
			--available;
		}
		DWORD index = 0;
		for (std::list<Gfx_Light*>::const_iterator entry = lights->begin();
			entry != lights->end(); ++entry)
		{
			Gfx_Light* light = *entry;
			if (g_GfxSystem->DebugLightsEnabled())
			{
				g_GfxSystem->SubmitLight(light, reinterpret_cast<Gfx_Viewport*>(viewport));
			}
			if (index < available)
			{
				if (!automap)
				{
					D3DLIGHT8 description;
					std::memcpy(&description, &light->light, sizeof(description));
					g_D3D.device->SetLight(index, &description);
					g_D3D.device->LightEnable(index, TRUE);
					++index;
				}
				else if (!light->value44)
				{
					D3DLIGHT8 description;
					std::memcpy(&description, &light->light, sizeof(description));
					g_D3D.device->SetLight(index, &description);
					g_D3D.device->LightEnable(index, TRUE);
					++index;
				}
			}
		}
		if (extra)
		{
			D3DLIGHT8 description;
			std::memset(&description, 0, sizeof(description));
			description.Type = D3DLIGHT_DIRECTIONAL;
			std::memcpy(&description.Ambient, extra, sizeof(description.Ambient));
			description.Direction.y = -1.0f;
			g_D3D.device->SetLight(index, &description);
			g_D3D.device->LightEnable(index, TRUE);
			++index;
		}
		for (; index < g_GfxSystem->MaximumActiveLights(); ++index)
		{
			g_D3D.device->LightEnable(index, FALSE);
		}
	}
	else if (extra)
	{
		D3DLIGHT8 description;
		std::memset(&description, 0, sizeof(description));
		std::memcpy(&description.Ambient, extra, sizeof(description.Ambient));
		description.Type = D3DLIGHT_DIRECTIONAL;
		description.Direction.y = -1.0f;
		g_D3D.device->SetLight(0, &description);
		g_D3D.device->LightEnable(0, TRUE);
		for (DWORD index = 1; index < g_GfxSystem->MaximumActiveLights(); ++index)
		{
			g_D3D.device->LightEnable(index, FALSE);
		}
	}
}

void Gfx_RS_Expand::Release()
{
	if (g_RSExpandState.users)
	{
		--g_RSExpandState.users;
		if (!g_RSExpandState.users && g_RSExpandState.effect)
		{
			g_RSExpandState.effect->Release();
			g_RSExpandState.effect = NULL;
		}
	}
}
