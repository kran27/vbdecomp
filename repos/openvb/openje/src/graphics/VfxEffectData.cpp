#include "VfxEffectData.hpp"
#include "StringFormat.hpp"
#include "VfxWeather.hpp"
#include "VfxParticles.hpp"
#include "VfxCollision.hpp"
#include "VfxSound.hpp"
#include "VfxSprite.hpp"
#include "VfxModelData.hpp"
#include "VfxLight.hpp"
#include "VfxTrail.hpp"
#include "VfxModelEffect.hpp"
#include "VfxMotionBlur.hpp"
#include "VfxWater.hpp"
#include <cstring>
#include <stdexcept>

VFXEFFECTTYPE_BASE_Struct::VFXEFFECTTYPE_BASE_Struct()
{
	// Only constructed member values initialize; primitive fields stay unwritten.
}

namespace JE
{
	VFX_ColorBytes::VFX_ColorBytes() //52E850.
	{
		values[0] = 0; values[1] = 0; values[2] = 0;
	}

	VFX_Vector2::VFX_Vector2() //560490.
	{
		values[0] = 0.0f;
		values[1] = 0.0f;
	}

	VFX_Vector2::VFX_Vector2(float x, float y) //52F780.
	{
		values[0] = x;
		values[1] = y;
	}

	VFX_Vector::VFX_Vector() : xy(), z(0.0f) {} //523B00.
	VFX_Vector::VFX_Vector(float x, float y, float z) : xy(x, y), z(z) {} //523B20.

	bool VFX_Vector::operator!=(const VFX_Vector& input) const //501150.
	{
		const bool equal = input.xy.values[0] == xy.values[0]
			&& input.xy.values[1] == xy.values[1] && input.z == z;
		return !equal;
	}

	VFX_TextureCoordinates::VFX_TextureCoordinates() //52EA80.
	{
		values[0] = 0.0f; values[1] = 1.0f;
		values[2] = 1.0f; values[3] = 0.0f;
	}

	VFX_TextureCoordinates::VFX_TextureCoordinates(float x, float y, float z, float w) //52EAA0.
	{
		values[0] = x; values[1] = y; values[2] = z; values[3] = w;
	}

	VFX_Rotation::VFX_Rotation() //52C260.
	{
		values[0] = 0.0f;
		values[1] = 0.0f;
		values[2] = 0.0f;
	}

	VFX_Rotation::VFX_Rotation(float x, float y, float z) //52C270.
	{
		values[0] = x;
		values[1] = y;
		values[2] = z;
	}

	VFX_Target::VFX_Target() : selector(0) {} //502110.
	VFX_Target::VFX_Target(DWORD value) : selector(value) {} //401450.
	VFX_Target& VFX_Target::operator=(DWORD value) //401450, in-place selector publication.
	{
		selector = value;
		return *this;
	}
	DWORD VFX_Target::Value() const { return selector; } //502120.

	void VFX_ParameterHolder::Clear()
	{
		parameters.clear();
		order.clear();
	}

	VFX_Parameter::VFX_Parameter()
		: type(Unknown), subtype(-1), valueLimit(-1), valueSize(0), value(NULL)
	{
	}

	void VFX_EffectInstance::LoadBaseData(const VFX_BaseData& input, float value) //4E1D50.
	{
		baseData = input;
		scale = value;
		effectIndex = DWORD(-1);
	}

	void ReadVfxBaseData(VFX_BaseData* output, const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if ((input = parameters.Find("Name")) != NULL)
		{
			const std::string& returned = input->GetString();
			output->name = std::string(returned);
		}
		if ((input = parameters.Find("EffectType")) != NULL) output->effectType = input->GetEnum();
		if ((input = parameters.Find("Delay")) != NULL) output->delay = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("Duration")) != NULL) output->duration = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("Target")) != NULL) output->target = input->GetTarget();
		if ((input = parameters.Find("Socket")) != NULL) output->socket = input->GetEnum();
		if ((input = parameters.Find("SnapBone")) != NULL)
		{
			const std::string& returned = input->ToString();
			output->snapBone = std::string(returned);
		}
		if ((input = parameters.Find("TargetBodyPart")) != NULL) output->targetBodyPart = input->GetBool();
		if ((input = parameters.Find("HideIfNoTarget")) != NULL) output->hideIfNoTarget = input->GetBool();
		if ((input = parameters.Find("Offset")) != NULL) output->offset = input->GetVector3();
		if ((input = parameters.Find("Rotation")) != NULL)
		{
			const VFX_Rotation rotation = input->GetRotation();
			output->rotation = rotation;
		}
		if ((input = parameters.Find("RotateWithTarget")) != NULL) output->rotateWithTarget = input->GetBool();
		if ((input = parameters.Find("TriggeredEffect")) != NULL) output->triggeredEffect = input->GetBool();
	}

	VFX_LightningData::VFX_LightningData()
	{
		// Native vector and RGB value owners initialize in member order.
		// The scalar/flag primitives not explicitly constructed at 0x4E9070
		// stay unwritten. Actual format-default merging supplies them later.
	}

	void VFX_LightningData::Read(const VFX_ParameterHolder& parameters)
	{
		const VFX_Parameter* input;
		if ((input = parameters.Find("RenderPhase")) != NULL)
			renderPhase = input->GetEnum();
		if ((input = parameters.Find("Modulate2x")) != NULL)
			modulate2x = input->GetBool();
		if ((input = parameters.Find("InnerGlow")) != NULL)
			innerGlow = input->GetBool();
		if ((input = parameters.Find("MinLifetime")) != NULL)
			minLifetime = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MaxLifetime")) != NULL)
			maxLifetime = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("RepeatCount")) != NULL)
			repeatCount = input->GetInteger();
		if ((input = parameters.Find("MinRepeatDelay")) != NULL)
			minRepeatDelay = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MaxRepeatDelay")) != NULL)
			maxRepeatDelay = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("EndPointTarget")) != NULL)
		{
			const VFX_Target target = input->GetTarget();
			endPointTarget = target;
		}
		if ((input = parameters.Find("EndPointSocket")) != NULL)
			endPointSocket = input->GetEnum();
		if ((input = parameters.Find("EndPointSnapBone")) != NULL)
			endPointSnapBone = input->GetString();
		if ((input = parameters.Find("EndPointTargetBodyPart")) != NULL)
			endPointTargetBodyPart = input->GetBool();
		if ((input = parameters.Find("EndPointOffset")) != NULL)
			{ const JE::VFX_Vector converted = input->GetVector3(); endPointOffset = converted; }
		if ((input = parameters.Find("MinSegmentsPerFoot")) != NULL)
			minSegmentsPerFoot = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MaxSegmentsPerFoot")) != NULL)
			maxSegmentsPerFoot = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MinAdditionalSegments")) != NULL)
			minAdditionalSegments = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MaxAdditionalSegments")) != NULL)
			maxAdditionalSegments = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MinJaggedness")) != NULL)
			minJaggedness = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MaxJaggedness")) != NULL)
			maxJaggedness = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MinMeander")) != NULL)
			minMeander = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MaxMeander")) != NULL)
			maxMeander = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("AlwaysStrikeTarget")) != NULL)
			alwaysStrikeTarget = input->GetBool();
		if ((input = parameters.Find("MinWidth")) != NULL)
			minWidth = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MaxWidth")) != NULL)
			maxWidth = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("WidthStyle")) != NULL)
			widthStyle = input->GetEnum();
		if ((input = parameters.Find("TipWidthScale")) != NULL)
			tipWidthScale = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("BranchProbability")) != NULL)
			branchProbability = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MinBranchAngle")) != NULL)
			minBranchAngle = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MaxBranchAngle")) != NULL)
			maxBranchAngle = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MaxBranches")) != NULL)
			maxBranches = input->GetInteger();
		if ((input = parameters.Find("MaxBranchLevels")) != NULL)
			maxBranchLevels = input->GetInteger();
		if ((input = parameters.Find("MinBranchWidthScale")) != NULL)
			minBranchWidthScale = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MaxBranchWidthScale")) != NULL)
			maxBranchWidthScale = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MinBranchLengthScale")) != NULL)
			minBranchLengthScale = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("MaxBranchLengthScale")) != NULL)
			maxBranchLengthScale = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("OuterColor")) != NULL)
			{ const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(outerColor, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("InnerColor")) != NULL)
			{ const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(innerColor, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("AdjustOuterColor")) != NULL)
			adjustOuterColor = input->GetBool();
		if ((input = parameters.Find("AdjustOuterColorOverLifetime")) != NULL)
			adjustOuterColorOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalOuterColor")) != NULL)
			{ const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(finalOuterColor, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("OuterColorWaveform")) != NULL)
			outerColorWaveform = input->GetEnum();
		if ((input = parameters.Find("OuterColorFreq")) != NULL)
			outerColorFreq = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("OuterColorPhase")) != NULL)
			outerColorPhase = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("AdjustInnerColor")) != NULL)
			adjustInnerColor = input->GetBool();
		if ((input = parameters.Find("AdjustInnerColorOverLifetime")) != NULL)
			adjustInnerColorOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalInnerColor")) != NULL)
			{ const JE::VFX_ColorBytes converted = input->GetColor(); memcpy(finalInnerColor, converted.values, sizeof(converted.values)); }
		if ((input = parameters.Find("InnerColorWaveform")) != NULL)
			innerColorWaveform = input->GetEnum();
		if ((input = parameters.Find("InnerColorFreq")) != NULL)
			innerColorFreq = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("InnerColorPhase")) != NULL)
			innerColorPhase = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("OuterAlpha")) != NULL)
			outerAlpha = static_cast<BYTE>(input->GetEnum());
		if ((input = parameters.Find("InnerAlpha")) != NULL)
			innerAlpha = static_cast<BYTE>(input->GetEnum());
		if ((input = parameters.Find("AdjustOuterAlpha")) != NULL)
			adjustOuterAlpha = input->GetBool();
		if ((input = parameters.Find("AdjustOuterAlphaOverLifetime")) != NULL)
			adjustOuterAlphaOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalOuterAlpha")) != NULL)
			finalOuterAlpha = static_cast<BYTE>(input->GetEnum());
		if ((input = parameters.Find("OuterAlphaWaveform")) != NULL)
			outerAlphaWaveform = input->GetEnum();
		if ((input = parameters.Find("OuterAlphaFreq")) != NULL)
			outerAlphaFreq = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("OuterAlphaPhase")) != NULL)
			outerAlphaPhase = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("AdjustInnerAlpha")) != NULL)
			adjustInnerAlpha = input->GetBool();
		if ((input = parameters.Find("AdjustInnerAlphaOverLifetime")) != NULL)
			adjustInnerAlphaOverLifetime = input->GetBool();
		if ((input = parameters.Find("FinalInnerAlpha")) != NULL)
			finalInnerAlpha = static_cast<BYTE>(input->GetEnum());
		if ((input = parameters.Find("InnerAlphaWaveform")) != NULL)
			innerAlphaWaveform = input->GetEnum();
		if ((input = parameters.Find("InnerAlphaFreq")) != NULL)
			innerAlphaFreq = static_cast<float>(input->GetFloat());
		if ((input = parameters.Find("InnerAlphaPhase")) != NULL)
			innerAlphaPhase = static_cast<float>(input->GetFloat());
	}

	void ReadVfxEffectData(VFX_BaseData* output, const VFX_ParameterHolder& parameters)
	{
		ReadVfxBaseData(output, parameters);
		const VFX_Parameter* type = parameters.Find("EffectType");
		if (!type) return;
		switch (type->GetEnum())
		{
		case 0: if (output) static_cast<VFX_ParticleData*>(output)->Read(parameters); break;
		case 1: if (output) static_cast<VFX_MotionBlurData*>(output)->Read(parameters); break;
		case 2: if (output) static_cast<VFX_SpriteData*>(output)->Read(parameters); break;
		case 3: if (output) static_cast<VFX_ModelData*>(output)->Read(parameters); break;
		case 4: if (output) static_cast<VFX_TrailData*>(output)->Read(parameters); break;
		case 5: if (output) static_cast<VFX_CollisionData*>(output)->Read(parameters); break;
		case 6: if (output) static_cast<VFX_WaterData*>(output)->Read(parameters); break;
		case 7: if (output) static_cast<VFX_LightningData*>(output)->Read(parameters); break;
		case 8: if (output) static_cast<VFX_ModelEffectData*>(output)->Read(parameters); break;
		case 9: if (output) static_cast<VFX_LightData*>(output)->Read(parameters); break;
		case 10: if (output) static_cast<VFX_RainData*>(output)->Read(parameters); break;
		case 11: if (output) static_cast<VFX_SnowData*>(output)->Read(parameters); break;
		case 12: if (output) static_cast<VFX_WindData*>(output)->Read(parameters); break;
		case 13: if (output) static_cast<VFX_SoundData*>(output)->Read(parameters); break;
		}
	}
	VFX_BaseData* BuildVfxEffectData(const VFX_ParameterHolder& parameters)
	{
		VFX_BaseData* base = new VFX_BaseData();
		const VFX_Parameter* type = parameters.Find("EffectType");
		if (type)
		{
			LONG selected = type->GetEnum();
			if (selected >= 0 && selected <= 13)
			{
				VFX_BaseData* specific = NULL;
				if (selected == 0) specific = new VFX_ParticleData;
				else if (selected == 1) specific = new VFX_MotionBlurData;
				else if (selected == 2) specific = new VFX_SpriteData;
				else if (selected == 3) specific = new VFX_ModelData;
				else if (selected == 4) specific = new VFX_TrailData;
				else if (selected == 5) specific = new VFX_CollisionData;
				else if (selected == 6) specific = new VFX_WaterData;
				else if (selected == 7) specific = new VFX_LightningData;
				else if (selected == 8) specific = new VFX_ModelEffectData;
				else if (selected == 9) specific = new VFX_LightData;
				else if (selected == 10) specific = new VFX_RainData;
				else if (selected == 11) specific = new VFX_SnowData;
				else if (selected == 12) specific = new VFX_WindData;
				else if (selected == 13) specific = new VFX_SoundData;
				else
				{
					delete base;
					throw std::logic_error("VFX factory 0x534890 specific effect data unresolved.");
				}
				ReadVfxEffectData(specific, parameters);
				delete base;
				return specific;
			}
		}
		// The original explicitly retains a base record for absent/unknown types.
		ReadVfxBaseData(base, parameters);
		return base;
	}
}

namespace JE
{
	VFX_Parameter& VFX_Parameter::operator=(const VFX_Parameter& input)
	{
		name = input.name;
		type = input.type;
		subtype = input.subtype;
		SetValueLimit(input.valueLimit);
		//Reload after SetValueLimit, including when input aliases this object.
		SetValue(input.value, input.valueSize);
		return *this;
	}

	VFX_ResourceData __cdecl ParseResourceValue(const char* input) //52E5B0.
	{
		VFX_ResourceData result;
		const std::string text(input ? input : "");
		char token[1024];
		const char* cursor = Tokenize(token, text.c_str(), 1024, " \t\n\r,;", 0, 1);
		result.type.assign(token);
		Tokenize(token, cursor, 1024, " \t\n\r,;", 0, 1);
		if (std::strlen(token) && token[0] == '"' && token[std::strlen(token) - 1] == '"')
		{
			token[0] = ' ';
			token[std::strlen(token) - 1] = ' ';
			const std::string quotedToken(token);
			// Separate owning input preserves TrimString's unwritten-output cases.
			TrimString(token, quotedToken.c_str(), " \t\n\r");
		}
		result.name.assign(token);
		return VFX_ResourceData(result);
	}

	VFX_ResourceData::VFX_ResourceData() {} //40A220.

	VFX_ResourceData::VFX_ResourceData(const BYTE* input)
	{
		DWORD offset = 0;
		Read(input, offset);
	}

	void VFX_Parameter::ResizeValue(LONG count)
	{
		if (valueSize != count)
		{
			if (value)
			{
				::operator delete[](value);
				value = NULL;
			}
			if (count <= 0)
			{
				valueSize = 0;
				return;
			}
			value = static_cast<BYTE*>(::operator new[](static_cast<size_t>(count)));
			valueSize = count;
		}
	}

	VFX_Parameter& VFX_Parameter::SetInteger(LONG input)
	{
		SetValue(&input, sizeof(input));
		type = Integer;
		return *this;
	}

	VFX_Parameter& VFX_Parameter::SetByte(BYTE input)
	{
		SetValue(&input, sizeof(input));
		type = Byte;
		return *this;
	}

	VFX_Parameter& VFX_Parameter::SetBool(bool input)
	{
		BYTE byte = input;
		SetValue(&byte, sizeof(byte));
		type = Boolean;
		return *this;
	}

	BOOL VFX_Parameter::GetBool() const
	{
		return GetInteger() != 0;
	}

	LONG VFX_Parameter::GetEnum() const
	{
		return GetInteger();
	}

	VFX_Parameter& VFX_Parameter::SetResource(const VFX_ResourceData& input)
	{
		DWORD count = 0;
		input.Write(NULL, count);
		if (static_cast<LONG>(count) > 0)
		{
			BYTE* bytes = static_cast<BYTE*>(::operator new[](count));
			if (bytes)
			{
				DWORD offset = 0;
				input.Write(bytes, offset);
				SetValue(bytes, count);
				::operator delete[](bytes);
			}
		}
		type = Resource;
		return *this;
	}

	VFX_Parameter& VFX_Parameter::SetString(const std::string& input)
	{
		SetValue(input.c_str(), input.size() + 1);
		type = String;
		return *this;
	}

	VFX_Parameter& VFX_Parameter::SetSnapBone(const std::string& input)
	{
		SetValue(input.c_str(), input.size() + 1);
		type = SnapBone;
		return *this;
	}

	VFX_Parameter& VFX_Parameter::SetFloat(float input)
	{
		SetValue(&input, sizeof(input));
		type = Float;
		return *this;
	}

	VFX_Parameter& VFX_Parameter::SetVector3(const VFX_Vector& input)
	{
		SetValue(&input, 12);
		type = Vector;
		return *this;
	}

	VFX_Parameter& VFX_Parameter::SetVector2(const float* input)
	{
		SetValue(input, 8);
		type = Vector2;
		return *this;
	}

	VFX_Parameter& VFX_Parameter::SetTextureUVs(const float* input)
	{
		SetValue(input, 16);
		type = TextureUVs;
		return *this;
	}

	VFX_Parameter& VFX_Parameter::SetRotation(const float* input)
	{
		SetValue(input, 12);
		type = Rotation;
		return *this;
	}

	VFX_Parameter& VFX_Parameter::SetColor(const BYTE* input)
	{
		SetValue(input, 3);
		type = Color;
		return *this;
	}

	VFX_Parameter& VFX_Parameter::SetTriggeredEffect(LONG input)
	{
		SetValue(&input, 4);
		type = TriggeredEffect;
		return *this;
	}
}
