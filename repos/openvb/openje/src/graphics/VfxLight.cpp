#include "VfxLight.hpp"
#include <string.h>
#include "ParticleSystem.hpp"
#include <float.h>
#include <new>
#include <stdexcept>

namespace
{
	double LightWaveform(BYTE relative, float duration, double normalizedTime,
		float currentTime, LONG waveform, float frequency, float phase)
	{
		double time = relative && duration > 0 ? normalizedTime : currentTime;
		float value = float(time * frequency + phase);
		return Gfx_ParticleWaveform(waveform, 0, 1, value); // 0x512F70.
	}



}

namespace JE
{
	VFX_LightData::VFX_LightData()
	{
		// Six native RGB value owners initialize in declaration order.
		// Other primitives are constructor-unwritten; format-default merging supplies them.
	}

	void VFX_LightData::Read(const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if ((input = parameters.Find("LightType")) != NULL) lightType = input->GetEnum();
		if ((input = parameters.Find("Direction")) != NULL) { const JE::VFX_Vector converted = input->GetVector3(); memcpy(&direction, &converted, sizeof(converted)); }
		if ((input = parameters.Find("Diffuse")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(diffuse, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("DiffusePercent")) != NULL) diffusePercent = float(input->GetFloat());
		if ((input = parameters.Find("Specular")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(specular, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("Ambient")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(ambient, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("Range")) != NULL) range = float(input->GetFloat());
		if ((input = parameters.Find("Falloff")) != NULL) falloff = float(input->GetFloat());
		if ((input = parameters.Find("Attenuation0")) != NULL) attenuation0 = float(input->GetFloat());
		if ((input = parameters.Find("Attenuation1")) != NULL) attenuation1 = float(input->GetFloat());
		if ((input = parameters.Find("Attenuation2")) != NULL) attenuation2 = float(input->GetFloat());
		if ((input = parameters.Find("Theta")) != NULL) theta = float(input->GetFloat());
		if ((input = parameters.Find("Phi")) != NULL) phi = float(input->GetFloat());
		if ((input = parameters.Find("Priority")) != NULL) priority = input->GetInteger();
		if ((input = parameters.Find("CastShadow")) != NULL) castShadow = input->GetBool();
		if ((input = parameters.Find("ShowShape")) != NULL) showShape = input->GetBool();
		if ((input = parameters.Find("AdjustDiffuse")) != NULL) adjustDiffuse = input->GetBool();
		if ((input = parameters.Find("AdjustDiffuseOverLifetime")) != NULL) adjustDiffuseOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalDiffuse")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(finalDiffuse, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("DiffuseWaveform")) != NULL) diffuseWaveform = input->GetEnum();
		if ((input = parameters.Find("DiffuseFreq")) != NULL) diffuseFreq = float(input->GetFloat());
		if ((input = parameters.Find("DiffusePhase")) != NULL) diffusePhase = float(input->GetFloat());
		if ((input = parameters.Find("AdjustSpecular")) != NULL) adjustSpecular = input->GetBool();
		if ((input = parameters.Find("AdjustSpecularOverLifetime")) != NULL) adjustSpecularOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalSpecular")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(finalSpecular, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("SpecularWaveform")) != NULL) specularWaveform = input->GetEnum();
		if ((input = parameters.Find("SpecularFreq")) != NULL) specularFreq = float(input->GetFloat());
		if ((input = parameters.Find("SpecularPhase")) != NULL) specularPhase = float(input->GetFloat());
		if ((input = parameters.Find("AdjustAmbient")) != NULL) adjustAmbient = input->GetBool();
		if ((input = parameters.Find("AdjustAmbientOverLifetime")) != NULL) adjustAmbientOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalAmbient")) != NULL) { const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(finalAmbient, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("AmbientWaveform")) != NULL) ambientWaveform = input->GetEnum();
		if ((input = parameters.Find("AmbientFreq")) != NULL) ambientFreq = float(input->GetFloat());
		if ((input = parameters.Find("AmbientPhase")) != NULL) ambientPhase = float(input->GetFloat());
		if ((input = parameters.Find("AdjustRange")) != NULL) adjustRange = input->GetBool();
		if ((input = parameters.Find("AdjustRangeOverLifetime")) != NULL) adjustRangeOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalRange")) != NULL) finalRange = float(input->GetFloat());
		if ((input = parameters.Find("RangeWaveform")) != NULL) rangeWaveform = input->GetEnum();
		if ((input = parameters.Find("RangeFreq")) != NULL) rangeFreq = float(input->GetFloat());
		if ((input = parameters.Find("RangePhase")) != NULL) rangePhase = float(input->GetFloat());
	}

	VFX_Light::VFX_Light()
	{
		currentTime = 0;
		light = NULL;
		influenceShape = NULL;
	}

	VFX_Light::~VFX_Light()
	{
		if (light) delete light;
		if (influenceShape) influenceShape->Release();
		// Embedded debugLines/data/base destruction follows in the original order.
	}

	VFX_Light& VFX_Light::CopyFrom(const VFX_Light& other) //0x4E83B0.
	{
		VFX_EffectInstance::CopyFrom(other);
		data = other.data;
		currentTime = other.currentTime;
		return *this;
	}
	void VFX_Light::SetData(const VFX_BaseData* input)
	{
		VFX_EffectInstance::SetData(input);
		data = *static_cast<const VFX_LightData*>(input); // 0x4E8160 typed member copy.
		if (light) { delete light; light = NULL; }
		D3DLIGHT8 value;
		Gfx_InitLightDescriptor(&value, D3DLIGHTTYPE(data.lightType + 1),
			data.diffuse[0] * data.diffusePercent * 0.000039215687f,
			data.diffuse[1] * data.diffusePercent * 0.000039215687f,
			data.diffuse[2] * data.diffusePercent * 0.000039215687f,
			data.specular[0] * data.diffusePercent * 0.000039215687f,
			data.specular[1] * data.diffusePercent * 0.000039215687f,
			data.specular[2] * data.diffusePercent * 0.000039215687f,
			data.ambient[0] * data.diffusePercent * 0.000039215687f,
			data.ambient[1] * data.diffusePercent * 0.000039215687f,
			data.ambient[2] * data.diffusePercent * 0.000039215687f,
			data.range * scale, data.falloff, data.attenuation0,
			data.attenuation1, data.attenuation2, data.theta, data.phi);
		const D3DXVECTOR3 direction(data.direction.xy.values[0], data.direction.xy.values[1], data.direction.z);
		value.Direction = direction;
		light = new Gfx_Light(value, 2);
		light->flag48 = data.castShadow;
		const D3DXVECTOR3 position(transform._41, transform._42, transform._43);
		light->Gfx_Light::SetPosition(position.x, position.y, position.z);
	}

	void VFX_Light::Reset() { active = 0; }
	BYTE VFX_Light::GetFlag() const { return 0; }
	BYTE VFX_Light::SkipBounds() const { return 0; }
	void VFX_Light::Draw() {} // World545AA0, not this virtual, supplies the light to the map renderer.

	double VFX_Light::NormalizedTime() const
	{
		double time = data.duration > 0 ? double(currentTime) / data.duration : 0;
		if (time < 0) return 0;
		return time > 1 ? 1 : time;
	}

	float VFX_Light::CurrentRange() const
	{
		if (!data.adjustRange) return data.range;
		double amount = LightWaveform(data.adjustRangeOverLifetime, data.duration, NormalizedTime(),
			currentTime, data.rangeWaveform, data.rangeFreq, data.rangePhase);
		return float((double(data.finalRange) - data.range) * amount + data.range);
	}

	void VFX_Light::Update(float time) { Update(time, VFX_UpdateContext()); }
	void VFX_Light::Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target, VFX_VEGInstance* group,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		VFX_EffectInstance::Update(time, source, target, group, sourceTransform, targetTransform);
		if (currentTime != time) currentTime = time;
		if (!light) return;
		float normalizedTime;
		if (data.duration > 0)
		{
			normalizedTime = currentTime / data.duration;
			if (normalizedTime < 0) normalizedTime = 0;
			else if (normalizedTime > 1) normalizedTime = 1;
		}
		else normalizedTime = 0;
		D3DXVECTOR3 direction(data.direction.xy.values[0], data.direction.xy.values[1], data.direction.z);
		// Original471357 transforms the same owned vector, including translation.
		D3DXVec3TransformCoord(&direction, &direction, &transform);
		Gfx_Light* transformedLight = light;
		const D3DXMATRIX matrix(transform);
		transformedLight->transform = matrix;
		transformedLight->light.Position.x = transform._41;
		transformedLight->light.Position.y = transform._42;
		transformedLight->light.Position.z = transform._43;
		light->light.Direction = direction;
		if (data.adjustRange)
		{
			const float phase = data.adjustRangeOverLifetime && data.duration > 0 ? normalizedTime : currentTime;
			const float argument = float(phase * data.rangeFreq + data.rangePhase);
			const double amount = Gfx_ParticleWaveform(data.rangeWaveform, 0, 1, argument);
			light->light.Range = float(((data.finalRange - data.range) * amount + data.range) * scale);
		}
		if (data.adjustDiffuse)
		{
			const float phase = data.adjustDiffuseOverLifetime && data.duration > 0 ? normalizedTime : currentTime;
			const float argument = float(phase * data.diffuseFreq + data.diffusePhase);
			const double amount = Gfx_ParticleWaveform(data.diffuseWaveform, 0, 1, argument);
			const float green = ((LONG(data.finalDiffuse[1]) - data.diffuse[1]) * amount + data.diffuse[1]) * data.diffusePercent * 0.000039215687f;
			const float blue = ((LONG(data.finalDiffuse[2]) - data.diffuse[2]) * amount + data.diffuse[2]) * data.diffusePercent * 0.000039215687f;
			const float red = ((LONG(data.finalDiffuse[0]) - data.diffuse[0]) * amount + data.diffuse[0]) * data.diffusePercent * 0.000039215687f;
			D3DCOLORVALUE& color = light->light.Diffuse;
			color.a = 1;
			color.r = float(red);
			color.g = float(green);
			color.b = float(blue);
		}
		if (data.adjustSpecular)
		{
			const float phase = data.adjustSpecularOverLifetime && data.duration > 0 ? normalizedTime : currentTime;
			const float argument = float(phase * data.specularFreq + data.specularPhase);
			const double amount = Gfx_ParticleWaveform(data.specularWaveform, 0, 1, argument);
			const float green = ((LONG(data.finalSpecular[1]) - data.specular[1]) * amount + data.specular[1]) * 0.0039215689f;
			const float blue = ((LONG(data.finalSpecular[2]) - data.specular[2]) * amount + data.specular[2]) * 0.0039215689f;
			const float red = ((LONG(data.finalSpecular[0]) - data.specular[0]) * amount + data.specular[0]) * 0.0039215689f;
			D3DCOLORVALUE& color = light->light.Specular;
			color.a = 1;
			color.r = float(red);
			color.g = float(green);
			color.b = float(blue);
		}
		if (data.adjustAmbient)
		{
			const float phase = data.adjustAmbientOverLifetime && data.duration > 0 ? normalizedTime : currentTime;
			const float argument = float(phase * data.ambientFreq + data.ambientPhase);
			const double amount = Gfx_ParticleWaveform(data.ambientWaveform, 0, 1, argument);
			const float green = ((LONG(data.finalAmbient[1]) - data.ambient[1]) * amount + data.ambient[1]) * 0.0039215689f;
			const float blue = ((LONG(data.finalAmbient[2]) - data.ambient[2]) * amount + data.ambient[2]) * 0.0039215689f;
			const float red = ((LONG(data.finalAmbient[0]) - data.ambient[0]) * amount + data.ambient[0]) * 0.0039215689f;
			D3DCOLORVALUE& color = light->light.Ambient;
			color.a = 1;
			color.r = float(red);
			color.g = float(green);
			color.b = float(blue);
		}
	}

	void VFX_Light::GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const
	{
		D3DXVECTOR3 center(transform._41, transform._42, transform._43);
		float phase;
		if (data.duration > 0)
		{
			phase = currentTime / data.duration;
			if (phase < 0) phase = 0;
			else if (phase > 1) phase = 1;
		}
		else phase = 0;
		float range = data.range;
		if (data.adjustRange)
		{
			if (!data.adjustRangeOverLifetime || !(data.duration > 0)) phase = currentTime;
			const float argument = phase * data.rangeFreq + data.rangePhase;
			const double amount = Gfx_ParticleWaveform(data.rangeWaveform, 0, 1, argument);
			range = float((data.finalRange - data.range) * amount + data.range);
		}
		float extent;
		if (data.lightType == 0) extent = range;
		else if (data.lightType == 1)
		{
			D3DXVECTOR3 spotCenter(transform._41, transform._42, transform._43);
			D3DXVECTOR3 direction(data.direction.xy.values[0], data.direction.xy.values[1], data.direction.z);
			D3DXVec3Normalize(&direction, &direction);
			extent = range * 0.5f;
			direction *= extent;
			spotCenter += direction;
			center = spotCenter;
		}
		else if (data.lightType == 2)
		{
			low.x = -FLT_MAX;
			low.y = -FLT_MAX;
			low.z = -FLT_MAX;
			high.x = FLT_MAX;
			high.y = FLT_MAX;
			high.z = FLT_MAX;
			return;
		}
		else
		{
			// Native4E7B20 interprets the incoming low-output pointer home as FLOAT.
			const D3DXVECTOR3* output = &low;
			memcpy(&extent, &output, sizeof extent);
		}
		const float radius = extent * scale;
		const float lowX = center.x - radius;
		low.x = lowX < FLT_MAX ? float(lowX) : FLT_MAX;
		const float lowY = center.y - radius;
		low.y = lowY < FLT_MAX ? float(lowY) : FLT_MAX;
		const float lowZ = center.z - radius;
		low.z = lowZ < FLT_MAX ? float(lowZ) : FLT_MAX;
		const float highX = center.x + radius;
		high.x = highX > -FLT_MAX ? float(highX) : -FLT_MAX;
		const float highY = center.y + radius;
		high.y = highY > -FLT_MAX ? float(highY) : -FLT_MAX;
		const float highZ = center.z + radius;
		high.z = highZ > -FLT_MAX ? float(highZ) : -FLT_MAX;
	}
}
