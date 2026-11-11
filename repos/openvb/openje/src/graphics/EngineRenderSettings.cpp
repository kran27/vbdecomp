#include "Engine.hpp"
#include "D3DState.hpp"
#include "GraphicsFiltering.hpp"
#include "RenderManager.hpp"
#include "DebugSphere.hpp"
#include "Light.hpp"
#include "Viewport.hpp"
#include "ViewportState.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

void Gfx_RenderManager::SubmitLight(Gfx_Light* light)
{
	if (!g_GfxSystem->DebugLightsEnabled()) return;
	debugLights.insert(std::make_pair(light, DWORD(0))).first->second = 1;
	Gfx_DebugSphere505880* sphere = GetDebugSphere();
	const D3DCOLORVALUE diffuse = light->light.Diffuse;
	const DWORD red = DWORD(static_cast<__int64>((diffuse.r + 1.0f) / 2.0f * 255.0f)) & 0xFF;
	const DWORD alpha = DWORD(static_cast<__int64>(diffuse.a * 255.0f));
	const DWORD green = DWORD(static_cast<__int64>((diffuse.g + 1.0f) / 2.0f * 255.0f)) & 0xFF;
	const DWORD blue = DWORD(static_cast<__int64>((diffuse.b + 1.0f) / 2.0f * 255.0f)) & 0xFF;
	sphere->SetColors(1, 1, (alpha << 24) | (red << 16) | (green << 8) | blue);
	const float range = light->light.Range;
	if (range > 0.0f)
	{
		Gfx_BasePacketData* const packet = sphere->packet;
		packet->scale.x = range;
		packet->scale.y = range;
		packet->scale.z = range;
		sphere->radius = range;
	}
	D3DXVECTOR3 position;
	std::memcpy(&position, &light->transform._41, sizeof(position));
	sphere->SetPosition(position.x, position.y, position.z);
	sphere->Submit(owner);
}

Gfx_DebugSphere505880* Gfx_RenderManager::GetDebugSphere()
{
	Gfx_DebugSphere505880* sphere;
	if (!freeDebugSpheres.empty())
	{
		sphere = freeDebugSpheres.front();
		freeDebugSpheres.pop_front();
	}
	else
	{
		sphere = new Gfx_DebugSphere505880(8, 16);
		if (!sphere) JE::FatalError("Gfx_RenderManager::GetDebugSphere - Out of memory!");
	}
	usedDebugSpheres.push_back(sphere);
	return sphere;
}

void Gfx_Engine::EnableProjectedShadow(BYTE value)
{
	if (value)
	{
		if (projectedShadowTexture)
		{
			projectedShadows = value;
			defaultViewport->renderer->flag49 = value;
		}
		else
		{
			JE::LogDebugString("Gfx_Engine::EnableProjectedShadow - ProjectedShadowTex and/or ShadowTex not available.\n");
		}
	}
	else
	{
		Gfx_Viewport* viewport = defaultViewport;
		projectedShadows = 0;
		viewport->renderer->flag49 = 0;
	}
}

BYTE Gfx_Engine::ProjectedShadowEnabled()
{
	return projectedShadows;
}

void Gfx_Engine::SetLinearFiltering(BYTE enabled)
{
	DWORD stage = 0;
	if (enabled)
	{
		while (stage < g_D3D.currentDevice->caps.MaxTextureBlendStages)
		{
			g_D3D.SetTextureStageState(stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			g_D3D.SetTextureStageState(stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			++stage;
		}
	}
	else
	{
		while (stage < g_D3D.currentDevice->caps.MaxTextureBlendStages)
		{
			g_D3D.SetTextureStageState(stage, D3DTSS_MAGFILTER, D3DTEXF_POINT);
			g_D3D.SetTextureStageState(stage, D3DTSS_MINFILTER, D3DTEXF_POINT);
			++stage;
		}
	}
}

void Gfx_Engine::SetMipFiltering(BYTE enabled)
{
	DWORD stage = 0;
	if (enabled)
	{
		while (stage < g_D3D.currentDevice->caps.MaxTextureBlendStages)
		{
			g_D3D.SetTextureStageState(stage, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
			++stage;
		}
	}
	else
	{
		while (stage < g_D3D.currentDevice->caps.MaxTextureBlendStages)
		{
			g_D3D.SetTextureStageState(stage, D3DTSS_MIPFILTER, D3DTEXF_NONE);
			++stage;
		}
	}
}

void Gfx_Engine::SetLighting(BYTE enabled)
{
	lighting = enabled;
	g_D3D.SetRenderState(D3DRS_LIGHTING, enabled);
}

BYTE Gfx_Engine::Lighting()
{
	return lighting;
}

void Gfx_Engine::SetAmbientRGB(BYTE red, BYTE green, BYTE blue)
{
	g_D3D.SetRenderState(D3DRS_AMBIENT,
		D3DCOLOR_XRGB(static_cast<char>(red), static_cast<char>(green), static_cast<char>(blue)));
}

void Gfx_Engine::SetAmbientColor(const D3DCOLOR* color)
{
	g_D3D.SetRenderState(D3DRS_AMBIENT, *color);
}

void Gfx_Engine::GetAmbientRGB(BYTE* red, BYTE* green, BYTE* blue)
{
	DWORD color;
	g_D3D.GetRenderState(D3DRS_AMBIENT, &color);
	*red = BYTE(color >> 16);
	*green = BYTE(color >> 8);
	*blue = BYTE(color);
}

void Gfx_Engine::GetAmbientColor(D3DCOLOR* color)
{
	g_D3D.GetRenderState(D3DRS_AMBIENT, color);
}

void Gfx_Engine::SetFogEnabled(BYTE enabled, Gfx_Viewport* viewport)
{
	Gfx_Viewport* target = defaultViewport;
	if (viewport) viewport->fogEnabled = enabled;
	else target->fogEnabled = enabled;
}

BYTE Gfx_Engine::FogEnabled(Gfx_Viewport* viewport)
{
	Gfx_Viewport* target = defaultViewport;
	if (viewport) target = viewport;
	return target->fogEnabled;
}

void Gfx_Engine::SetFogColor(const D3DCOLOR* color, Gfx_Viewport* viewport)
{
	Gfx_Viewport* target = defaultViewport;
	if (viewport) viewport->fogColor = *color;
	else target->fogColor = *color;
}

D3DCOLOR Gfx_Engine::FogColor(Gfx_Viewport* viewport)
{
	Gfx_Viewport* target = defaultViewport;
	if (viewport) target = viewport;
	return target->fogColor;
}

void Gfx_Engine::SetFogRange(float start, float end, Gfx_Viewport* viewport)
{
	Gfx_Viewport* target = defaultViewport;
	if (viewport) target = viewport;
	target->fogStart = start;
	target->fogEnd = end;
}

void Gfx_Engine::GetFogRange(float* start, float* end, Gfx_Viewport* viewport)
{
	Gfx_Viewport* target = defaultViewport;
	if (viewport) target = viewport;
	std::memcpy(start, &target->fogStart, sizeof(*start));
	std::memcpy(end, &target->fogEnd, sizeof(*end));
}

float Gfx_Engine::FogDensity(Gfx_Viewport* viewport)
{
	Gfx_Viewport* target = defaultViewport;
	if (viewport) target = viewport;
	const double result = target->fogDensity;
	return float(result);
}

void Gfx_Engine::SetFogDensity(float density, Gfx_Viewport* viewport)
{
	Gfx_Viewport* target = defaultViewport;
	if (viewport) std::memcpy(&viewport->fogDensity, &density, sizeof(density));
	else std::memcpy(&target->fogDensity, &density, sizeof(density));
}

void Gfx_Engine::CreateHeightFog(D3DCOLOR color, float baseHeight, DWORD value8,
	DWORD valueC, float totalHeight, DWORD value14, DWORD value18,
	DWORD renderValue, float maxDensity, Gfx_Viewport* viewport)
{
	//5BE010 forwards nine words unchanged to490330; the DWORD interface
	//parameters carry floating-point bits, not numeric integer values.

	Gfx_Viewport* target = defaultViewport;
	if (viewport) target = viewport;
	target->renderer->CreateHeightFog(color, baseHeight,
		*reinterpret_cast<const float*>(&value8), *reinterpret_cast<const float*>(&valueC),
		totalHeight, *reinterpret_cast<const float*>(&value14),
		*reinterpret_cast<const float*>(&value18), *reinterpret_cast<const float*>(&renderValue),
		maxDensity);
}

void Gfx_Engine::DestroyHeightFog(Gfx_Viewport* viewport)
{
	Gfx_Viewport* target = defaultViewport;
	if (viewport) target = viewport;
	target->renderer->ReleaseHeightFog();
}

BYTE Gfx_Engine::EnableHeightFog(BYTE enabled, Gfx_Viewport* viewport)
{
	Gfx_Viewport* target = defaultViewport;
	if (viewport) target = viewport;
	return target->renderer->EnableHeightFog(enabled);
}

BYTE Gfx_Engine::HeightFogEnabled(Gfx_Viewport* viewport)
{
	Gfx_Viewport* target = defaultViewport;
	if (viewport) target = viewport;
	return target->renderer->flag40;
}

void Gfx_Engine::SetFogOfWarEnabled(BYTE enabled, Gfx_Viewport* viewport)
{
	Gfx_Viewport* target = defaultViewport;
	if (viewport) target = viewport;
	target->renderer->flag48 = enabled;
	fogOfWar = enabled;
}

BYTE Gfx_Engine::FogOfWarEnabled(Gfx_Viewport* viewport)
{
	Gfx_Viewport* target = defaultViewport;
	if (viewport) target = viewport;
	return target->renderer->flag48;
}

void Gfx_Engine::SetFlag2194(BYTE enabled)
{
	revealExplored = enabled;
}

BYTE Gfx_Engine::Flag2194()
{
	return revealExplored;
}

void Gfx_Engine::SetRendererColor(D3DCOLOR color, Gfx_Viewport* viewport)
{
	if (!viewport)
	{
		defaultViewport->renderer->shadowColor = color;
		for (std::list<Gfx_Viewport*>::iterator i = additionalViewports.begin();
			i != additionalViewports.end(); ++i)
			(*i)->renderer->shadowColor = color;
	}
	else viewport->renderer->shadowColor = color;
}

D3DCOLOR Gfx_Engine::RendererColor(Gfx_Viewport* viewport)
{
	if (!viewport) return defaultViewport->renderer->shadowColor;
	return viewport->renderer->shadowColor;
}

void Gfx_Engine::SetRendererOwner76(void* owner, Gfx_Viewport* viewport)
{
	if (!viewport) defaultViewport->renderer->floor = static_cast<Octree2Gfx*>(owner);
	else viewport->renderer->floor = static_cast<Octree2Gfx*>(owner);
}

void Gfx_Engine::SetProjectedShadowTexture(DWORD handle)
{
	if (handle)
	{
		if (LookupTexture(handle)) projectedShadowTexture = handle;
		else JE::LogDebugString("Gfx_Engine::SetProjectedShadowTexture - Unable to find texture!\n");
	}
	else
	{
		JE::LogDebugString("Gfx_Engine::SetProjectedShadowTexture - Setting a NULL texture!\n");
	}
}

void Gfx_Engine::AddProjectedShadowPoint(const float* xyz)
{
	if (!projectedShadows) return;
	if (shadowPointCount < shadowPointCapacity)
	{
		std::memcpy(shadowPoints + shadowPointCount, xyz, 12);
		++shadowPointCount;
	}
	else
	{
		JE::LogDebugString("Gfx_Engine::AddProjectedShadowPoint - Need larger buffer!!! (current size: %d)\n",
			shadowPointCapacity);
	}
}

void Gfx_Engine::SetSilhouetteEdgeTexture(DWORD handle)
{
	if (handle)
	{
		if (LookupTexture(handle)) textureF4 = handle;
		else JE::LogDebugString("Gfx_Engine::SetSilhouetteEdgeTexture - Unable to find texture!\n");
	}
	else
	{
		JE::LogDebugString("Gfx_Engine::SetSilhouetteEdgeTexture - Setting a NULL texture!\n");
	}
}

DWORD Gfx_Engine::SilhouetteEdgeTexture()
{
	return textureF4;
}

void Gfx_Engine::SetFlag2188(BYTE value)
{
	statistics = value;
}

void Gfx_Engine::SetAlphaTest(BYTE enabled)
{
	if (enabled)
	{
		g_D3D.SetRenderState(D3DRS_ALPHATESTENABLE, 1);
		g_D3D.SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	}
	else
	{
		g_D3D.SetRenderState(D3DRS_ALPHATESTENABLE, 0);
		g_D3D.SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
	}
	transparency = enabled;
}

void Gfx_Engine::SetWireframe(BYTE enabled)
{
	if (enabled) g_D3D.SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else g_D3D.SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
}

void Gfx_Engine::SetDebugLightsEnabled(BYTE value)
{
	value890 = value;
}

BYTE Gfx_Engine::DebugLightsEnabled()
{
	return value890;
}

void Gfx_Engine::SubmitLight(Gfx_Light* light, Gfx_Viewport* viewport)
{
	if (viewport) viewport->renderer->SubmitLight(light);
	else defaultViewport->renderer->SubmitLight(light);
}

void Gfx_Engine::SetRendererFlag116(BYTE value, Gfx_Viewport* viewport)
{
	Gfx_RenderManager* renderer;
	if (viewport) renderer = viewport->renderer;
	else renderer = defaultViewport->renderer;
	renderer->flag74 = value;
}
