#include "VfxWater.hpp"
#include <string.h>
#include "Water.hpp"
#include "WaterTile.hpp"
#include "TextureResource.hpp"
#include <float.h>
#include <new>
#include <stdlib.h>
#include <stdexcept>

namespace JE
{
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char VfxWaterResourceSize[sizeof(VFX_ResourceData) == 56 ? 1 : -1];
	typedef char VfxWaterDataSize[sizeof(VFX_WaterData) == 416 ? 1 : -1];
	typedef char VfxWaterInstanceSize[sizeof(VFX_Water) == 628 ? 1 : -1];
#endif

	VFX_WaterData::VFX_WaterData() //4EE150.
	{
		// Native UV and RGB value owners initialize in member order.
		//All other scalar/flag fields stay unwritten until their parameter is present.
	}

	void VFX_WaterData::Read(const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if ((input = parameters.Find("Texture")) != NULL)
		{
			texture.type = input->GetResourceType();
			texture.name = input->GetResourceName();
		}
		if ((input = parameters.Find("TextureUVs")) != NULL) { const JE::VFX_TextureCoordinates converted = input->GetTextureUVs(); memcpy(textureUVs, &converted, sizeof(converted)); }
		if ((input = parameters.Find("LightMap")) != NULL)
		{
			lightMap.type = input->GetResourceType();
			lightMap.name = input->GetResourceName();
		}
		if ((input = parameters.Find("LightMapUVs")) != NULL) { const JE::VFX_TextureCoordinates converted = input->GetTextureUVs(); memcpy(lightMapUVs, &converted, sizeof(converted)); }
		if ((input = parameters.Find("Width")) != NULL) width = input->GetInteger();
		if ((input = parameters.Find("Height")) != NULL) height = input->GetInteger();
		if ((input = parameters.Find("GridSize")) != NULL) gridSize = float(input->GetFloat());
		if ((input = parameters.Find("Color")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(color, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("RenderPhase")) != NULL) renderPhase = input->GetEnum();
		if ((input = parameters.Find("DoubleSided")) != NULL) doubleSided = input->GetBool();
		if ((input = parameters.Find("Modulate2x")) != NULL) modulate2x = input->GetBool();
		if ((input = parameters.Find("IgnoreLighting")) != NULL) ignoreLighting = input->GetBool();
		if ((input = parameters.Find("UpdateInterval")) != NULL) updateInterval = float(input->GetFloat());
		if ((input = parameters.Find("Alpha")) != NULL) alpha = BYTE(input->GetEnum());
		if ((input = parameters.Find("AdjustAlpha")) != NULL) adjustAlpha = input->GetBool();
		if ((input = parameters.Find("AdjustAlphaOverLifetime")) != NULL)
			adjustAlphaOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalAlpha")) != NULL) finalAlpha = BYTE(input->GetEnum());
		if ((input = parameters.Find("AlphaWaveform")) != NULL) alphaWaveform = input->GetEnum();
		if ((input = parameters.Find("AlphaFreq")) != NULL) alphaFreq = float(input->GetFloat());
		if ((input = parameters.Find("AlphaPhase")) != NULL) alphaPhase = float(input->GetFloat());
		if ((input = parameters.Find("WaterMotionType")) != NULL) waterMotionType = input->GetEnum();
		if ((input = parameters.Find("MinDropLocation")) != NULL) { const JE::VFX_Vector2 converted = input->GetVector2(); memcpy(&minDropLocation, &converted, sizeof(converted)); }
		if ((input = parameters.Find("MaxDropLocation")) != NULL) { const JE::VFX_Vector2 converted = input->GetVector2(); memcpy(&maxDropLocation, &converted, sizeof(converted)); }
		if ((input = parameters.Find("MinDropAmount")) != NULL) minDropAmount = float(input->GetFloat());
		if ((input = parameters.Find("MaxDropAmount")) != NULL) maxDropAmount = float(input->GetFloat());
		if ((input = parameters.Find("MinDropInterval")) != NULL) minDropInterval = float(input->GetFloat());
		if ((input = parameters.Find("MaxDropInterval")) != NULL) maxDropInterval = float(input->GetFloat());
		if ((input = parameters.Find("MinFlowOrigin")) != NULL) { const JE::VFX_Vector2 converted = input->GetVector2(); memcpy(&minFlowOrigin, &converted, sizeof(converted)); }
		if ((input = parameters.Find("MaxFlowOrigin")) != NULL) { const JE::VFX_Vector2 converted = input->GetVector2(); memcpy(&maxFlowOrigin, &converted, sizeof(converted)); }
		if ((input = parameters.Find("MinFlowDirection")) != NULL) { const JE::VFX_Vector2 converted = input->GetVector2(); memcpy(&minFlowDirection, &converted, sizeof(converted)); }
		if ((input = parameters.Find("MaxFlowDirection")) != NULL) { const JE::VFX_Vector2 converted = input->GetVector2(); memcpy(&maxFlowDirection, &converted, sizeof(converted)); }
		if ((input = parameters.Find("MinFlowAmount")) != NULL) minFlowAmount = float(input->GetFloat());
		if ((input = parameters.Find("MaxFlowAmount")) != NULL) maxFlowAmount = float(input->GetFloat());
		if ((input = parameters.Find("MinFlowInterval")) != NULL) minFlowInterval = float(input->GetFloat());
		if ((input = parameters.Find("MaxFlowInterval")) != NULL) maxFlowInterval = float(input->GetFloat());
		if ((input = parameters.Find("MinSwirlRadius")) != NULL) minSwirlRadius = float(input->GetFloat());
		if ((input = parameters.Find("MaxSwirlRadius")) != NULL) maxSwirlRadius = float(input->GetFloat());
		if ((input = parameters.Find("MinSwirlSpeed")) != NULL) minSwirlSpeed = float(input->GetFloat());
		if ((input = parameters.Find("MaxSwirlSpeed")) != NULL) maxSwirlSpeed = float(input->GetFloat());
		if ((input = parameters.Find("MinSwirlAmount")) != NULL) minSwirlAmount = float(input->GetFloat());
		if ((input = parameters.Find("MaxSwirlAmount")) != NULL) maxSwirlAmount = float(input->GetFloat());
		if ((input = parameters.Find("MinSwirlInterval")) != NULL) minSwirlInterval = float(input->GetFloat());
		if ((input = parameters.Find("MaxSwirlInterval")) != NULL) maxSwirlInterval = float(input->GetFloat());
	}

	void __stdcall BuildVfxWater(Gfx_Water** water, Gfx_WaterTile** tile,
		const VFX_WaterData& data) //4EDD20.
	{
		*water = new Gfx_Water;
		*tile = new Gfx_WaterTile;
		if (!*water || !*tile) return;
		(*water)->flag174 = 1;
		(*water)->sampleWidth = data.width;
		(*water)->sampleHeight = data.height;
		(*water)->cellSize = data.gridSize;
		(*water)->tilesX = data.width - 1 < 0 ? 0 : data.width - 1;
		(*water)->tilesZ = data.height - 1 < 0 ? 0 : data.height - 1;
		(*water)->updateInterval = data.updateInterval;
		(*water)->disturbanceMode = data.waterMotionType;
		switch (data.waterMotionType)
		{
		case 1:
			(*water)->SetRippleRange(
				D3DXVECTOR2(data.minDropLocation.values[0], data.minDropLocation.values[1]),
				D3DXVECTOR2(data.maxDropLocation.values[0], data.maxDropLocation.values[1]),
				data.minDropAmount, data.maxDropAmount, data.minDropInterval, data.maxDropInterval);
			break;
		case 2:
			(*water)->SetLineRange(
				D3DXVECTOR2(data.minFlowOrigin.values[0], data.minFlowOrigin.values[1]),
				D3DXVECTOR2(data.maxFlowOrigin.values[0], data.maxFlowOrigin.values[1]),
				D3DXVECTOR2(data.minFlowDirection.values[0], data.minFlowDirection.values[1]),
				D3DXVECTOR2(data.maxFlowDirection.values[0], data.maxFlowDirection.values[1]), data.minFlowAmount,
				data.maxFlowAmount, data.minFlowInterval, data.maxFlowInterval);
			break;
		case 3:
			(*water)->SetOrbitRange(data.minSwirlRadius, data.maxSwirlRadius,
				data.minSwirlSpeed, data.maxSwirlSpeed, data.minSwirlAmount,
				data.maxSwirlAmount, data.minSwirlInterval, data.maxSwirlInterval);
			break;
		}
		(*water)->PrepareSamples();
		(*tile)->directWater = *water;
		(*tile)->SetTexture(0, 0);
		(*tile)->SetLightmap(0, 0);
		(*tile)->columns = data.width;
		(*tile)->rows = data.height;
		(*tile)->offsetX = (*tile)->offsetZ = 0;
		(*tile)->mode28 = data.renderPhase;
		(*tile)->flag224 = data.doubleSided == 0;
		(*tile)->flag225 = data.modulate2x;
		(*tile)->flag226 = data.ignoreLighting;
		(*tile)->red = data.color[0];
		(*tile)->green = data.color[1];
		(*tile)->blue = data.color[2];
		(*tile)->alpha = data.alpha;
		(*tile)->animateAlpha = data.adjustAlpha;
		(*tile)->normalizeAlphaTime = data.adjustAlphaOverLifetime;
		(*tile)->alphaEnd = data.finalAlpha;
		(*tile)->alphaWaveform = data.alphaWaveform;
		(*tile)->alphaFrequency = data.alphaFreq;
		(*tile)->alphaPhase = data.alphaPhase;
		(*tile)->alphaDuration = data.duration;
		(*tile)->value23C = data.updateInterval;
		(*tile)->Rebuild();
		(*tile)->SetSurfaceUV(data.textureUVs[0], data.textureUVs[1],
			data.textureUVs[2] - data.textureUVs[0], data.textureUVs[3] - data.textureUVs[1]);
		(*tile)->SetTextureUV(data.lightMapUVs[0], data.lightMapUVs[1],
			data.lightMapUVs[2] - data.lightMapUVs[0], data.lightMapUVs[3] - data.lightMapUVs[1]);
	}

	VFX_Water::VFX_Water() //4EE390.
	{
		water = NULL;
		tile = NULL;
		lastTime = NULL;
		value270 = 1.0f;
	}

	VFX_Water::~VFX_Water() //4EE300.
	{
		if (water) delete water;
		if (tile) delete tile;
		if (lastTime) delete lastTime;
	}
	void VFX_Water::Load(const VFX_BaseData* input, float value) //4ED0E0.
	{
		LoadBaseData(*input, value);
		if (input)
		{
			value270 = value;
			SetData(*input);
		}
	}

	VFX_Water& VFX_Water::CopyFrom(const VFX_Water& other) //4EE6F0.
	{
		if (water) delete water;
		if (tile) delete tile;
		VFX_EffectInstance::CopyFrom(other);
		data = other.data;
		water = NULL;
		tile = NULL;
		BuildVfxWater(&water, &tile, data);
		value270 = other.value270;
		if (other.water->textureHandle) water->SetTexture(other.water->textureHandle, 0);
		if (other.tile->texture) tile->SetTexture(other.tile->texture, 0);
		if (other.tile->lightmap) tile->SetLightmap(other.tile->lightmap, 0);
		return *this;
	}

	void VFX_Water::SetData(const VFX_BaseData* input) //4EE7C0.
	{
		VFX_EffectInstance::SetData(input);
		data = *static_cast<const VFX_WaterData*>(input);
		if (water) { delete water; water = NULL; }
		if (tile) { delete tile; tile = NULL; }
		BuildVfxWater(&water, &tile, *static_cast<const VFX_WaterData*>(input));
		if (water && tile)
		{
			const VFX_WaterData& source = *static_cast<const VFX_WaterData*>(input);
			if (!source.texture.name.empty())
				tile->SetTexture(Gfx_RequestTextureResource(source.texture.name.c_str(), 1, 1, 5, 0), 1);
			if (!source.lightMap.name.empty())
				tile->SetLightmap(Gfx_RequestTextureResource(source.lightMap.name.c_str(), 1, 1, 5, 0), 1);
		}
	}

	void VFX_Water::Reset() //4EDCE0.
	{
		active = 0;
		if (lastTime) { delete lastTime; lastTime = NULL; }
		if (water) water->Reset();
	}
	BYTE VFX_Water::GetFlag() const { return 1; } //Original slot1C returns AL1.
	BYTE VFX_Water::SkipBounds() const { return 0; }

	void VFX_Water::Update(float time)
	{
		Update(time, VFX_UpdateContext());
	}
	void VFX_Water::Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		if (!water || !tile) return;
		float elapsed = 0;
		if (lastTime) elapsed = time - *lastTime;
		else lastTime = new float;
		*lastTime = time;
		VFX_EffectInstance::Update(time, source, target, group, sourceTransform, targetTransform);
		if (IsVisible())
		{
			water->Update(elapsed);
			tile->Update(time);
		}
	}
	void VFX_Water::Draw() { Draw(NULL); }
	void VFX_Water::Draw(Gfx_Viewport* viewport) //4ED1C0.
	{
		if (water && tile) tile->Draw(viewport);
	}
	void VFX_Water::SetTransform(const D3DXMATRIX& input) //4ED1E0.
	{
		transform = input;
		if (tile)
		{
			D3DXMATRIX value = input;
			tile->extraTransform = value;
		}
	}
	void VFX_Water::GetTransform(D3DXMATRIX& output) const //4ED230.
	{
		output = tile ? tile->extraTransform : transform;
	}
	BYTE VFX_Water::AddLight(Gfx_Light* light) //4EDC80.
	{
		if (tile) return tile->AddLight(light);
		return 0;
	}
	void VFX_Water::RemoveLight(Gfx_Light* light) //4EDCA0.
	{
		if (tile) tile->RemoveLight(light);
	}
	void VFX_Water::ClearLights() //4EDCC0.
	{
		if (tile) tile->ClearLights();
	}
	void VFX_Water::FinishLights() //4EDCD0.
	{
		if (tile) tile->FinishLights();
	}

#define VFX_WATER_MIN(a, b) ((a) < (b) ? (a) : (b))
#define VFX_WATER_MAX(a, b) ((a) > (b) ? (a) : (b))
#define VFX_WATER_MIN4(a, b, c, d) VFX_WATER_MIN(a, VFX_WATER_MIN(b, VFX_WATER_MIN(c, VFX_WATER_MIN(d, FLT_MAX))))
#define VFX_WATER_MAX4(a, b, c, d) VFX_WATER_MAX(a, VFX_WATER_MAX(b, VFX_WATER_MAX(c, VFX_WATER_MAX(d, -FLT_MAX))))
	void VFX_Water::GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const //4ED260.
	{
		if (water)
		{
			const LONG width = water->tilesX;
			const float grid = water->cellSize;
			const LONG height = water->tilesZ;
			const float extentX = width * grid;
			const float extentZ = height * grid;
			D3DXVECTOR3 p0(0, 0, 0);
			D3DXVECTOR3 p1(extentX, 0, 0);
			D3DXVECTOR3 p2(0, 0, extentZ);
			D3DXVECTOR3 p3(extentX, 0, extentZ);
			D3DXVec3TransformCoord(&p0, &p0, &transform);
			D3DXVec3TransformCoord(&p1, &p1, &transform);
			D3DXVec3TransformCoord(&p2, &p2, &transform);
			D3DXVec3TransformCoord(&p3, &p3, &transform);
			low.x = VFX_WATER_MIN4(p0.x, p1.x, p2.x, p3.x);
			low.y = VFX_WATER_MIN4(p0.y, p1.y, p2.y, p3.y);
			low.z = VFX_WATER_MIN4(p0.z, p1.z, p2.z, p3.z);
			high.x = VFX_WATER_MAX4(p0.x, p1.x, p2.x, p3.x);
			high.y = VFX_WATER_MAX4(p0.y, p1.y, p2.y, p3.y);
			high.z = VFX_WATER_MAX4(p0.z, p1.z, p2.z, p3.z);
		}
		else VFX_EffectInstance::GetBounds(low, high);
	}
#undef VFX_WATER_MAX4
#undef VFX_WATER_MIN4
#undef VFX_WATER_MAX
#undef VFX_WATER_MIN
}
