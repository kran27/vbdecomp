#include "VfxParameters.hpp"
#include "StringFormat.hpp"
#include <d3dx8.h>
#include <mbstring.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <new>
#include <stdexcept>

namespace
{
	const char* parameterTypes[] =
	{
		"int", "VFX_Byte", "float", "bool", "enum", "std::string",
		"VFX_Vector", "VFX_Vector2", "VFX_Rotation", "VFX_Color",
		"VFX_Resource", "VFX_Target", "VFX_TextureUVs", "VFX_SnapBone",
		"VFX_TriggeredEffect"
	};

	const char* enumTypes[] =
	{
		"VFX_EffectType", "GFX_BONE_ID", "GFX_RENDER_PHASE", "VFX_WaveType",
		"VFX_AlignmentType", "VFX_EmitterShape", "VFX_CollisionShape",
		"VFX_CollisionType", "VFX_ParticleTrailType", "VFX_WaterMotionType",
		"VFX_LightningWidthStyle", "VFX_LightType", "VFX_ShadowVolumeType",
		"VFX_AABBType", "VFX_WindType"
	};

	const char* const enumValues0[] =
	{
		"VFXEFFECTTYPE_PARTICLESYSTEM",
		"VFXEFFECTTYPE_MOTIONBLUR",
		"VFXEFFECTTYPE_SPRITE",
		"VFXEFFECTTYPE_MODEL",
		"VFXEFFECTTYPE_TRAIL",
		"VFXEFFECTTYPE_COLLISION",
		"VFXEFFECTTYPE_WATER",
		"VFXEFFECTTYPE_LIGHTNING",
		"VFXEFFECTTYPE_MODELEFFECT",
		"VFXEFFECTTYPE_LIGHT",
		"VFXEFFECTTYPE_RAIN",
		"VFXEFFECTTYPE_SNOW",
		"VFXEFFECTTYPE_WIND",
		"VFXEFFECTTYPE_SOUND"
	};

	const char* const enumValues1[] =
	{
		"BONE_NONE",
		"BONE_ROOT",
		"BONE_PELVIS",
		"BONE_R_HIP",
		"BONE_R_THIGH",
		"BONE_R_CALF",
		"BONE_R_FOOT",
		"BONE_R_TOE",
		"BONE_L_HIP",
		"BONE_L_THIGH",
		"BONE_L_CALF",
		"BONE_L_FOOT",
		"BONE_L_TOE",
		"BONE_SPINE_1",
		"BONE_SPINE_2",
		"BONE_NECK",
		"BONE_HEAD",
		"BONE_R_CLAVICLE",
		"BONE_R_CLAVICLE_1",
		"BONE_R_CLAVICLE_2",
		"BONE_R_UPPERARM",
		"BONE_R_FOREARM",
		"BONE_R_WRIST",
		"BONE_R_HAND",
		"BONE_R_FINGER",
		"BONE_R_FINGER_1",
		"BONE_R_FINGER_2",
		"BONE_R_FINGER_INDEX",
		"BONE_R_FINGER_POINT",
		"BONE_R_FINGER_PINKY",
		"BONE_R_HANDLE_1",
		"BONE_R_HANDLE_2",
		"BONE_L_CLAVICLE",
		"BONE_L_CLAVICLE_1",
		"BONE_L_CLAVICLE_2",
		"BONE_L_UPPERARM",
		"BONE_L_FOREARM",
		"BONE_L_WRIST",
		"BONE_L_HAND",
		"BONE_L_FINGER",
		"BONE_L_FINGER_1",
		"BONE_L_FINGER_2",
		"BONE_L_FINGER_INDEX",
		"BONE_L_FINGER_POINT",
		"BONE_L_FINGER_PINKY",
		"BONE_L_HANDLE_1",
		"BONE_L_HANDLE_2",
		"BONE_PONYTAIL1",
		"BONE_PONYTAIL2",
		"BONE_PONYTAIL3",
		"BONE_CLOAK_1",
		"BONE_CLOAK_2",
		"BONE_CLOAK_3",
		"BONE_CLOAK_4",
		"BONE_JAW",
		"BONE_R_FOREHIP",
		"BONE_R_FORETHIGH",
		"BONE_R_FORECALF",
		"BONE_R_FOREFOOT",
		"BONE_R_FORETOES",
		"BONE_L_FOREHIP",
		"BONE_L_FORETHIGH",
		"BONE_L_FORECALF",
		"BONE_L_FOREFOOT",
		"BONE_L_FORETOES",
		"BONE_R_HINDHIP",
		"BONE_R_HINDTHIGH",
		"BONE_R_HINDCALF",
		"BONE_R_HINDFOOT",
		"BONE_R_HINDTOES",
		"BONE_L_HINDHIP",
		"BONE_L_HINDTHIGH",
		"BONE_L_HINDCALF",
		"BONE_L_HINDFOOT",
		"BONE_L_HINDTOES",
		"BONE_1_HIP",
		"BONE_2_HIP",
		"BONE_3_HIP",
		"BONE_1_THIGH",
		"BONE_2_THIGH",
		"BONE_3_THIGH",
		"BONE_1_CALF",
		"BONE_2_CALF",
		"BONE_3_CALF",
		"BONE_1_FOOT",
		"BONE_2_FOOT",
		"BONE_3_FOOT",
		"BONE_1_TOES",
		"BONE_2_TOES",
		"BONE_3_TOES",
		"BONE_SPINE_3",
		"BONE_1_CLAVICLE",
		"BONE_2_CLAVICLE",
		"BONE_3_CLAVICLE",
		"BONE_1_UPPERARM",
		"BONE_2_UPPERARM",
		"BONE_3_UPPERARM",
		"BONE_1_FOREARM",
		"BONE_2_FOREARM",
		"BONE_3_FOREARM",
		"BONE_1_HAND",
		"BONE_2_HAND",
		"BONE_3_HAND",
		"BONE_1_FINGERS",
		"BONE_2_FINGERS",
		"BONE_3_FINGERS",
		"BONE_1_MOUTH_0",
		"BONE_1_MOUTH_1",
		"BONE_2_MOUTH_0",
		"BONE_2_MOUTH_1",
		"BONE_3_MOUTH_0",
		"BONE_3_MOUTH_1",
		"BONE_1_NECK",
		"BONE_2_NECK",
		"BONE_3_NECK",
		"BONE_JAW_1",
		"BONE_JAW_2",
		"BONE_L_HINDHIP_2",
		"BONE_L_HINDTHIGH_2",
		"BONE_L_HINDCALF_2",
		"BONE_L_HINDFOOT_2",
		"BONE_L_HINDTOES_2",
		"BONE_R_HINDHIP_2",
		"BONE_R_HINDTHIGH_2",
		"BONE_R_HINDCALF_2",
		"BONE_R_HINDFOOT_2",
		"BONE_R_HINDTOES_2",
		"BONE_L_HINDHIP_3",
		"BONE_L_HINDTHIGH_3",
		"BONE_L_HINDCALF_3",
		"BONE_L_HINDFOOT_3",
		"BONE_L_HINDTOES_3",
		"BONE_R_HINDHIP_3",
		"BONE_R_HINDTHIGH_3",
		"BONE_R_HINDCALF_3",
		"BONE_R_HINDFOOT_3",
		"BONE_R_HINDTOES_3",
		"BONE_L_MANDIBLE_1",
		"BONE_L_MANDIBLE_2",
		"BONE_L_MANDIBLE_3",
		"BONE_R_MANDIBLE_1",
		"BONE_R_MANDIBLE_2",
		"BONE_R_MANDIBLE_3",
		"BONE_TAIL_1",
		"BONE_TAIL_2",
		"BONE_TAIL_3",
		"BONE_R_BROW",
		"BONE_L_BROW",
		"BONE_L_THUMB_1",
		"BONE_L_THUMB_2",
		"BONE_R_THUMB_1",
		"BONE_R_THUMB_2",
		"BONE_L_WING_1",
		"BONE_L_WING_2",
		"BONE_L_WING_3",
		"BONE_R_WING_1",
		"BONE_R_WING_2",
		"BONE_R_WING_3",
		"BONE_L_TENTCL_1",
		"BONE_R_TENTCL_1",
		"BONE_L_TENTCL_2",
		"BONE_R_TENTCL_2",
		"BONE_L_TENTCL_3",
		"BONE_R_TENTCL_3",
		"BONE_L_TENTCL_4",
		"BONE_R_TENTCL_4",
		"BONE_L_TENTCL_5",
		"BONE_R_TENTCL_5",
		"BONE_L_TENTCL_6",
		"BONE_R_TENTCL_6",
		"BONE_L_BRAIN",
		"BONE_R_BRAIN",
		"BONE_WEAPON_1",
		"BONE_WEAPON_2",
		"BONE_WEAPON_3",
		"BONE_WEAPON_4",
		"BONE_FX_MUZZLE",
		"BONE_FX_BRASS",
		"BONE_TREE_01",
		"BONE_TREE_02",
		"BONE_TREE_03",
		"BONE_TREE_04",
		"BONE_01",
		"BONE_02",
		"BONE_03",
		"BONE_04",
		"BONE_05",
		"BONE_06",
		"BONE_07",
		"BONE_08",
		"BONE_09",
		"BONE_10",
		"BONE_11",
		"BONE_12",
		"BONE_13",
		"BONE_14",
		"BONE_15",
		"BONE_16",
		"BONE_17",
		"BONE_18",
		"BONE_19",
		"BONE_20",
		"BONE_21",
		"BONE_22",
		"BONE_23",
		"BONE_24",
		"BONE_25",
		"BONE_26",
		"BONE_27",
		"BONE_28",
		"BONE_29",
		"BONE_30"
	};

	const char* const enumValues2[] =
	{
		"RP_OPAQUE",
		"RP_TRANSLUSCENT",
		"RP_BLEND_MULTIPLY",
		"RP_BLEND_SUBTRACT",
		"RP_BLEND_REVSUBTRACT",
		"RP_BLEND_ADD"
	};

	const char* const enumValues3[] =
	{
		"VFXWAVE_NONE",
		"VFXWAVE_LINEAR",
		"VFXWAVE_SQUARE",
		"VFXWAVE_SAW",
		"VFXWAVE_SINE"
	};

	const char* const enumValues4[] =
	{
		"VFXALIGN_ORIGIN",
		"VFXALIGN_CENTER",
		"VFXALIGN_ENDPOINT"
	};

	const char* const enumValues5[] =
	{
		"VFXEMITTERSHAPE_BOX",
		"VFXEMITTERSHAPE_SPHERE"
	};

	const char* const enumValues6[] =
	{
		"VFXCOLLISIONSHAPE_BOX",
		"VFXCOLLISIONSHAPE_SPHERE"
	};

	const char* const enumValues7[] =
	{
		"VFXCOLLISIONTYPE_BOUNCE",
		"VFXCOLLISIONTYPE_STICK",
		"VFXCOLLISIONTYPE_ERASE",
		"VFXCOLLISIONTYPE_ATTRACT",
		"VFXCOLLISIONTYPE_REPEL"
	};

	const char* const enumValues8[] =
	{
		"VFXPARTICLETRAILTYPE_NONE",
		"VFXPARTICLETRAILTYPE_AFTERIMAGE",
		"VFXPARTICLETRAILTYPE_RIBBON",
		"VFXPARTICLETRAILTYPE_LINE"
	};

	const char* const enumValues9[] =
	{
		"VFXWATERMOTIONTYPE_NONE",
		"VFXWATERMOTIONTYPE_RANDOMDROPS",
		"VFXWATERMOTIONTYPE_FLOW",
		"VFXWATERMOTIONTYPE_SWIRL"
	};

	const char* const enumValues10[] =
	{
		"VFXLIGHTNINGWIDTHSTYLE_MINTOMAX",
		"VFXLIGHTNINGWIDTHSTYLE_MAXTOMIN",
		"VFXLIGHTNINGWIDTHSTYLE_RANDOM"
	};

	const char* const enumValues11[] =
	{
		"VFXLIGHTTYPE_POINT",
		"VFXLIGHTTYPE_SPOT",
		"VFXLIGHTTYPE_DIRECTIONAL"
	};

	const char* const enumValues12[] =
	{
		"VFXSHADOWVOLUME_DEFAULT",
		"VFXSHADOWVOLUME_OFF",
		"VFXSHADOWVOLUME_ON"
	};

	const char* const enumValues13[] =
	{
		"VFXAABB_OPTIMIZED",
		"VFXAABB_AUTOMATIC",
		"VFXAABB_FIXED"
	};

	const char* const enumValues14[] =
	{
		"VFXWIND_CONSTANT",
		"VFXWIND_RANDOMGUSTS"
	};

	const char* const* const enumValues[] =
	{
		enumValues0, enumValues1, enumValues2, enumValues3, enumValues4,
		enumValues5, enumValues6, enumValues7, enumValues8, enumValues9,
		enumValues10, enumValues11, enumValues12, enumValues13, enumValues14
	};

	const LONG enumCounts[] = { 14, 212, 6, 5, 3, 2, 2, 5, 4, 4, 3, 3, 3, 3, 2 };
	const char* const targetNames[] =
	{
		"VFXTARGET_NONE", "VFXTARGET_SELF", "VFXTARGET_TARGET", "VFXTARGET_EFFECT"
	};

	template<class T> T Read(const BYTE* buffer, DWORD& offset)
	{
		T result;
		memcpy(&result, buffer + offset, sizeof(result));
		offset += sizeof(result);
		return result;
	}

	//Returned-value adapter for convenience accessors; not original4B8480.
	std::string ReadStringValue(const BYTE* buffer, DWORD& offset)
	{
		std::string result;
		JE::ReadVfxString(result, buffer, offset);
		return result;
	}

	void WriteString(std::vector<BYTE>& output, const std::string& input)
	{
		WORD length = static_cast<WORD>(input.size());
		const BYTE* count = reinterpret_cast<const BYTE*>(&length);
		output.insert(output.end(), count, count + sizeof(length));
		output.insert(output.end(), input.begin(), input.end());
	}

	//Preserve the original owned copy and returned-temporary destruction before
	//writing. A const-reference cast alone is copy-elided by the supplied VC7.
	const std::string& StringReference(const std::string& input) { return input; }
	const JE::VFX_ResourceData& ResourceReference(const JE::VFX_ResourceData& input) { return input; }

	// The 0x4A0FD0 token path used by type/enum/value parsers. Preserve leading
	// separators in the first token and skip them only after that token.
	const char* Token(char* output, const char* input)
	{
		*output = 0;
		if (!input) return NULL;
		const char* separators = " \t\n\r,;";
		DWORD count = 0;
		BYTE started = 0;
		while (*input)
		{
			char current = *input++;
			if (started)
			{
				if (strchr(separators, current)) break;
			}
			else started = strchr(separators, current) == NULL;
			output[count++] = current;
			if (count >= 1024)
			{
				// Original buffer terminates by overwriting the last byte.
				--count;
				break;
			}
		}
		output[count] = 0;
		while (*input && strchr(separators, *input)) ++input;
		return input;
	}

	LONG FindString(const char* input, const char* const* values, LONG count)
	{
		if (!input) return -1;
		for (LONG i = 0; i < count; ++i)
			if (!_mbscmp(reinterpret_cast<const unsigned char*>(input),
				reinterpret_cast<const unsigned char*>(values[i]))) return i;
		return -1;
	}

	template<class T> T Value(const JE::VFX_Parameter& parameter)
	{
		if (parameter.ValueSize() < sizeof(T))
			throw std::logic_error("VFX_Parameter value is truncated; original access is invalid.");
		T result;
		memcpy(&result, parameter.RawValue(), sizeof(result));
		return result;
	}

	//52E4E0: cdecl two-string formatting; preserve its owned result copy.
	std::string FormatResourceValue(const JE::VFX_ResourceData& input)
	{
		std::string result;
		JE::FormatString(result, "%s \"%s\"", input.type.c_str(), input.name.c_str());
		return std::string(result);
	}

	//502380: the payload is held across the borrowed-name/owned-label work.
	std::string FormatTargetValue(const BYTE* source)
	{
		std::string result;
		const JE::VFX_Target& input = *reinterpret_cast<const JE::VFX_Target*>(source);
		if (static_cast<LONG>(input.Value()) >= 3) //5023D0 is signed JL.
		{
			const std::string label(JE::VFXTargetLabelValue(3));
			const DWORD value = input.Value(); //Reload after owned-label construction.
			JE::FormatString(result, "%s %d", label.c_str(), static_cast<int>(value - 3));
		}
		else
		{
			const std::string label(JE::VFXTargetLabelValue(static_cast<LONG>(input.Value())));
			JE::FormatString(result, "%s", label.c_str());
		} //Destroy the label before constructing the returned copy.
		return std::string(result);
	}

	void TextVector(float* output, DWORD count, const std::string& input)
	{
		char token[1024];
		const char* cursor = input.c_str();
		for (DWORD i = 0; i < count; ++i)
		{
			cursor = Token(token, cursor);
			output[i] = static_cast<float>(atof(token));
		}
	}


}

namespace JE
{
	LONG FindVFXTargetLabel(const std::string& input)
	{
		for (LONG index = 0; index < 4; ++index)
		{
			const unsigned char* value = reinterpret_cast<const unsigned char*>(input.c_str());
			const unsigned char* label = reinterpret_cast<const unsigned char*>(targetNames[index]);
			if (!_mbscmp(label, value)) return index;
		}
		return 0;
	}

	std::string VFXTargetLabelValue(LONG value)
	{
		if (value >= 0 && value < 4) return std::string(targetNames[value]);
		return std::string("");
	}

	LONG FindVFXEnumerationValue(LONG subtype, const char* input)
	{
		if (input && subtype >= 0 && static_cast<DWORD>(subtype) < 15)
		{
			const LONG count = enumCounts[subtype];
			for (LONG index = 0; index < count; ++index)
				if (!_mbscmp(reinterpret_cast<const unsigned char*>(input),
					reinterpret_cast<const unsigned char*>(enumValues[subtype][index]))) return index;
		}
		return -1;
	}

	std::string VFXEnumerationValueLabel(LONG subtype, LONG value)
	{
		if (subtype >= 0 && value >= 0)
		{
			if (static_cast<DWORD>(subtype) >= 15 || value >= enumCounts[subtype])
				return std::string("");
			return std::string(enumValues[subtype][value]);
		}
		return std::string("");
	}

	BYTE VFX_ResourceData::Read(const BYTE* buffer, DWORD& offset)
	{
		ReadVfxString(type, buffer, offset);
		ReadVfxString(name, buffer, offset);
		return 1;
	}

	BYTE VFX_ResourceData::Write(BYTE* buffer, DWORD& offset) const
	{
		WriteVfxString(type, buffer, offset);
		WriteVfxString(name, buffer, offset);
		return 1;
	}

	VFX_Parameter::VFX_Parameter(const VFX_Parameter& input)
		: type(Unknown), subtype(-1), valueLimit(0), valueSize(0), value(NULL)
	{
		*this = input;
	}

	VFX_Parameter::~VFX_Parameter()
	{
		ResizeValue(0);
	}

	void VFX_Parameter::SetValueLimit(LONG limit)
	{
		LONG currentLimit = valueLimit;
		const LONG previousLimit = currentLimit;
		BYTE* saved = NULL;
		if (currentLimit > 0 && value)
		{
			saved = static_cast<BYTE*>(::operator new[](static_cast<size_t>(currentLimit)));
			//Both fields are reloaded after allocation; the count is the cap,
			//not valueSize. Preserve the original unchecked copy domain.
			memcpy(saved, value, static_cast<size_t>(valueLimit));
		}
		valueLimit = limit;
		ResizeValue(limit);
		currentLimit = valueLimit;
		if (currentLimit > 0)
		{
			BYTE* currentValue = value;
			if (currentValue && previousLimit > 0)
			{
				if (!saved) goto clamp;
				if (currentLimit >= previousLimit) currentLimit = previousLimit;
				memcpy(currentValue, saved, static_cast<size_t>(currentLimit));
			}
		}
		//No exception cleanup was present if ResizeValue throws above.
		if (saved) ::operator delete[](saved);
	clamp:
		if (valueSize > valueLimit) ResizeValue(valueLimit);
	}

	void VFX_Parameter::SetValue(const void* input, LONG count)
	{
		if (input && count > 0 && valueLimit != 0)
		{
			LONG limit = valueLimit;
			if (limit > 0) ResizeValue(count < limit ? count : limit);
			else ResizeValue(count);
			memcpy(value, input, static_cast<size_t>(valueSize));
		}
		else ResizeValue(0);
	}

	const BYTE* VFX_Parameter::RawValue() const
	{
		return value;
	}

	DWORD VFX_Parameter::ValueSize() const
	{
		return static_cast<DWORD>(valueSize);
	}

	void VFX_Parameter::RequireValue(DWORD count) const
	{
		if (ValueSize() < count)
			throw std::logic_error("VFX_Parameter value is truncated; original access is invalid.");
	}

	std::string VFX_Parameter::ValueString() const
	{
		if (!value) return std::string();
		const void* end = memchr(RawValue(), 0, ValueSize());
		if (!end)
			throw std::logic_error("VFX_Parameter string lacks the original required terminator.");
		return std::string(reinterpret_cast<const char*>(RawValue()));
	}

	BYTE VFX_Parameter::ReadParameter(const BYTE* buffer, DWORD& offset)
	{
		std::string inputName;
		std::string description;
		ReadVfxString(inputName, buffer, offset);
		ReadVfxString(description, buffer, offset);
		LONG limit; //Original prefix read has no invented count-only default.
		ReadVfxBuffer(&limit, 4, 1, buffer, offset);
		name.assign(inputName, 0, std::string::npos);
		type = GetTypeID(description.c_str());
		subtype = GetSubtypeID(description.c_str());
		valueLimit = limit;
		DWORD payloadSize = 0;
		ReadVfxBuffer(&payloadSize, 4, 1, buffer, offset);
		DWORD valueOffset = offset;
		ReadValue(buffer, valueOffset);
		//The declared extent wins; reload the caller's offset after the typed reader.
		offset += payloadSize;
		return 1;
	}

	std::string VFX_Parameter::GetEnumTypeName(LONG subtype)
	{
		if (subtype >= 0 && static_cast<DWORD>(subtype) < 15)
			return std::string(enumTypes[subtype]);
		return std::string("");
	}

	std::string VFX_Parameter::GetTypeDescription(LONG type, LONG subtype)
	{
		std::string result;
		if (type >= 0 && type < 15)
		{
			if (type != Enumeration)
				result = parameterTypes[type];
			else
			{
				result = parameterTypes[Enumeration];
				result.append(" ");
				result.append(GetEnumTypeName(subtype));
			}
		}
		return std::string(result);
	}

	BYTE VFX_Parameter::WriteParameter(BYTE* buffer, DWORD& offset) const
	{
		const std::string parameterName(name);
		const LONG parameterSubtype = subtype;
		const LONG parameterType = type;
		const std::string description = GetTypeDescription(parameterType, parameterSubtype);
		const LONG limit = valueLimit;
		WriteVfxString(parameterName, buffer, offset);
		WriteVfxString(description, buffer, offset);
		WriteVfxBuffer(&limit, 4, 1, buffer, offset);
		DWORD bytes = 0;
		if (buffer)
		{
			WriteValue(NULL, bytes);
			WriteVfxBuffer(&bytes, 4, 1, buffer, offset);
			WriteValue(buffer, offset);
		}
		else
		{
			WriteVfxBuffer(&bytes, 4, 1, NULL, offset);
			WriteValue(NULL, offset);
		}
		return 1;
	}

	void VFX_Parameter::WriteValue(BYTE* buffer, DWORD& offset) const
	{
		switch (type)
		{
		case Integer:
		{
			const LONG input = GetInteger();
			WriteVfxBuffer(&input, 4, 1, buffer, offset);
			break;
		}
		case Byte:
		{
			const BYTE input = static_cast<BYTE>(GetEnum());
			WriteVfxBuffer(&input, 1, 1, buffer, offset);
			break;
		}
		case Float:
		{
			const float input = static_cast<float>(GetFloat());
			WriteVfxBuffer(&input, 4, 1, buffer, offset);
			break;
		}
		case Boolean:
		{
			const BYTE input = GetBool() ? 0xDC : 0;
			WriteVfxBuffer(&input, 1, 1, buffer, offset);
			break;
		}
		case Enumeration:
		{
			const std::string input(StringReference(GetString()));
			WriteVfxString(input, buffer, offset);
			break;
		}
		case String:
		{
			const std::string input(StringReference(GetString()));
			WriteVfxString(input, buffer, offset);
			break;
		}
		case Vector:
		{
			const VFX_Vector converted = GetVector3();
			DWORD input[3];
			memcpy(input, &converted, sizeof(input));
			WriteVfxBuffer(input, 12, 1, buffer, offset);
			break;
		}
		case Vector2:
		{
			const VFX_Vector2 converted = GetVector2();
			DWORD input[2];
			memcpy(input, &converted, sizeof(input));
			WriteVfxBuffer(input, 8, 1, buffer, offset);
			break;
		}
		case Rotation:
		{
			const VFX_Rotation converted = GetRotation();
			DWORD input[3];
			memcpy(input, converted.values, sizeof(input));
			WriteVfxBuffer(input, 12, 1, buffer, offset);
			break;
		}
		case Color:
		{
			const VFX_ColorBytes converted = GetColor();
			BYTE input[3];
			memcpy(input, converted.values, sizeof(input));
			WriteVfxBuffer(input, 3, 1, buffer, offset);
			break;
		}
		case Resource:
		{
			const VFX_ResourceData input(ResourceReference(GetTypeAndValue()));
			input.Write(buffer, offset);
			break;
		}
		case Target:
		{
			const std::string input(StringReference(GetString()));
			WriteVfxString(input, buffer, offset);
			break;
		}
		case TextureUVs:
		{
			const VFX_TextureCoordinates converted = GetTextureUVs();
			DWORD input[4];
			memcpy(input, &converted, sizeof(input));
			WriteVfxBuffer(input, 16, 1, buffer, offset);
			break;
		}
		case SnapBone:
		{
			const std::string input(StringReference(ToString()));
			WriteVfxString(input, buffer, offset);
			break;
		}
		case TriggeredEffect:
		{
			const LONG input = GetEnum();
			WriteVfxBuffer(&input, 4, 1, buffer, offset);
			break;
		}
		default:
			break;
		}
	}

	void VFX_Parameter::ReadValue(const BYTE* buffer, DWORD& offset)
	{
		switch (type)
		{
		case Integer:
			{
				LONG input;
				ReadVfxBuffer(&input, 4, 1, buffer, offset);
				SetInteger(input);
			}
			break;
		case Byte:
			{
				BYTE input;
				ReadVfxBuffer(&input, 1, 1, buffer, offset);
				SetByte(input);
			}
			break;
		case Float:
			{
				float input;
				ReadVfxBuffer(&input, 4, 1, buffer, offset);
				SetFloat(input);
			}
			break;
		case Boolean:
			{
				BYTE input;
				ReadVfxBuffer(&input, 1, 1, buffer, offset);
				SetBool(input != 0);
			}
			break;
		case Enumeration:
			{
				std::string input;
				ReadVfxString(input, buffer, offset);
				SetText(input.c_str(), type, subtype);
			}
			break;
		case String:
			{
				std::string input;
				ReadVfxString(input, buffer, offset);
				SetString(input);
			}
			break;
		case Vector:
			{
				VFX_Vector input;
				ReadVfxBuffer(&input, 12, 1, buffer, offset);
				SetVector3(input);
			}
			break;
		case Vector2:
			{
				VFX_Vector2 input;
				ReadVfxBuffer(&input, 8, 1, buffer, offset);
				SetVector2(input.values);
			}
			break;
		case Rotation:
			{
				VFX_Rotation input;
				ReadVfxBuffer(input.values, 12, 1, buffer, offset);
				SetRotation(input.values);
			}
			break;
		case Color:
			{
				VFX_ColorBytes input;
				ReadVfxBuffer(input.values, 3, 1, buffer, offset);
				SetColor(input.values);
			}
			break;
		case Resource:
			{
				VFX_ResourceData input;
				input.Read(buffer, offset);
				SetResource(input);
			}
			break;
		case Target:
			{
				std::string input;
				ReadVfxString(input, buffer, offset);
				SetText(input.c_str(), type, subtype);
			}
			break;
		case TextureUVs:
			{
				VFX_TextureCoordinates input;
				ReadVfxBuffer(input.values, 16, 1, buffer, offset);
				SetTextureUVs(input.values);
			}
			break;
		case SnapBone:
			{
				std::string input;
				ReadVfxString(input, buffer, offset);
				SetSnapBone(input);
			}
			break;
		case TriggeredEffect:
			{
				LONG input; //Native leaves this value unwritten when buffer is NULL.
				ReadVfxBuffer(&input, 4, 1, buffer, offset);
				SetTriggeredEffect(input);
			}
			break;
		default:
			break; // Original does not read or alter the value for an unknown type.
		}
	}

	LONG VFX_Parameter::GetInteger() const
	{
		DWORD offset = 0;
		LONG result = 0;
		switch (type)
		{
		case Integer: case Enumeration: case TriggeredEffect:
			result = Read<LONG>(RawValue(), offset);
			break;
		case Target:
			result = static_cast<LONG>(reinterpret_cast<const VFX_Target*>(RawValue())->Value());
			break;
		case Byte:
			result = Read<BYTE>(RawValue(), offset);
			break;
		case Boolean:
			result = Read<BYTE>(RawValue(), offset) != 0;
			break;
		case Float: case Vector: case Vector2: case Rotation: case TextureUVs:
			result = static_cast<LONG>(static_cast<__int64>(Read<float>(RawValue(), offset)));
			break;
		case Color:
		{
			const BYTE* input = RawValue();
			result = input[0] | (input[1] << 8) | (input[2] << 16);
			break;
		}
		case String: case SnapBone:
			result = atol(reinterpret_cast<const char*>(RawValue()));
			break;
		case Resource:
		{
			const VFX_ResourceData input(RawValue());
			result = atol(input.name.c_str());
			break;
		}
		default:
			break;
		}
		return result;
	}

	double VFX_Parameter::GetFloat() const
	{
		DWORD offset = 0;
		double result = 0.0;
		switch (type)
		{
		case Integer: case Enumeration: case TriggeredEffect:
			result = Read<LONG>(RawValue(), offset);
			break;
		case Target:
			result = static_cast<LONG>(reinterpret_cast<const VFX_Target*>(RawValue())->Value());
			break;
		case Byte:
			result = Read<BYTE>(RawValue(), offset);
			break;
		case Boolean:
			result = Read<BYTE>(RawValue(), offset) != 0;
			break;
		case Float: case Vector: case Vector2: case Rotation: case TextureUVs:
			result = Read<float>(RawValue(), offset);
			break;
		case Color:
		{
			const BYTE* input = RawValue();
			result = static_cast<DWORD>(input[0] | (input[1] << 8) | (input[2] << 16));
			break;
		}
		case String: case SnapBone:
			result = atof(reinterpret_cast<const char*>(RawValue()));
			break;
		case Resource:
		{
			float converted;
			{
				const VFX_ResourceData input(RawValue());
				converted = static_cast<float>(atof(input.name.c_str()));
			} //4FE37C rounds before destroying both strings, then reloads the float.
			result = converted;
			break;
		}
		default:
			break;
		}
		return result;
	}

	std::string VFX_Parameter::GetString() const
	{
		DWORD offset = 0;
		std::string result;
		LONG integerValue;
		switch (type)
		{
		case Byte:
			integerValue = Read<BYTE>(RawValue(), offset);
			goto formatInteger;
		case Float:
			result.assign(JE::FormatFloat(Read<float>(RawValue(), offset), 3),
				0, std::string::npos);
			break;
		case Boolean:
			FormatString(result, "%d", Read<BYTE>(RawValue(), offset) != 0);
			break;
		case Enumeration:
		{
			LONG input = Read<LONG>(RawValue(), offset);
			result.assign(VFXEnumerationValueLabel(subtype, input), 0, std::string::npos);
			break;
		}
		case String:
		{
			const BYTE* input = RawValue();
			result.assign(input ? reinterpret_cast<const char*>(input) : "");
			break;
		}
		case Vector:
		{
			D3DXVECTOR3 input;
			memcpy(&input, RawValue(), sizeof(input));
			result.assign(FormatVector3(input, 3), 0, std::string::npos);
			break;
		}
		case Vector2:
		{
			D3DXVECTOR2 input;
			memcpy(&input, RawValue(), sizeof(input));
			result.assign(FormatVector2(input, 3), 0, std::string::npos);
			break;
		}
		case Rotation:
		{
			D3DXVECTOR3 input;
			memcpy(&input, RawValue(), sizeof(input));
			result.assign(FormatRotation(input, 3), 0, std::string::npos);
			break;
		}
		case Color:
		{
			result.assign(FormatColor(*reinterpret_cast<const VFX_ColorBytes*>(RawValue())),
				0, std::string::npos);
			break;
		}
		case Resource:
		{
			const VFX_ResourceData input(RawValue());
			result.assign(FormatResourceValue(input), 0, std::string::npos);
			break;
		}
		case Target:
			result.assign(FormatTargetValue(RawValue()), 0, std::string::npos);
			break;
		case TextureUVs:
		{
			D3DXVECTOR4 input;
			memcpy(&input, RawValue(), sizeof(input));
			result.assign(FormatUVs(input, 5), 0, std::string::npos);
			break;
		}
		case SnapBone:
		{
			const BYTE* input = RawValue();
			result.assign(input ? reinterpret_cast<const char*>(input) : "");
			break;
		}
		case Integer: case TriggeredEffect:
			integerValue = Read<LONG>(RawValue(), offset);
		formatInteger:
			FormatString(result, "%d", integerValue);
			break;
		default:
			break; //Original leaves the result empty for an unknown type.
		}
		return std::string(result); //5000C1 copies after case temporaries are destroyed.
	}

	std::string VFX_Parameter::ToString() const //500150, distinct from4FFD10.
	{
		std::string result;
		LONG integerValue;
		switch (type)
		{
		case Byte:
			integerValue = *value;
			goto formatInteger;
		case Float:
			result.assign(FormatFloat(*reinterpret_cast<const float*>(value), 3),
				0, std::string::npos);
			break;
		case Boolean:
			FormatString(result, "%d", *value != 0);
			break;
		case Enumeration:
		{
			const LONG input = *reinterpret_cast<const LONG*>(value);
			const LONG enumSubtype = subtype;
			result.assign(VFXEnumerationValueLabel(enumSubtype, input), 0, std::string::npos);
			break;
		}
		case String:
		{
			const BYTE* input = value;
			result.assign(input ? reinterpret_cast<const char*>(input) : "");
			break;
		}
		case Vector:
		{
			D3DXVECTOR3 input;
			memcpy(&input, value, sizeof(input));
			result.assign(FormatVector3(input, 3),
				0, std::string::npos);
			break;
		}
		case Vector2:
		{
			D3DXVECTOR2 input;
			memcpy(&input, value, sizeof(input));
			result.assign(FormatVector2(input, 3), 0, std::string::npos);
			break;
		}
		case Rotation:
		{
			D3DXVECTOR3 input;
			memcpy(&input, value, sizeof(input));
			result.assign(FormatRotation(input, 3),
				0, std::string::npos);
			break;
		}
		case Color:
		{
			result.assign(FormatColor(*reinterpret_cast<const VFX_ColorBytes*>(value)),
				0, std::string::npos);
			break;
		}
		case Resource:
		{
			const VFX_ResourceData input(value);
			result.assign(FormatResourceValue(input), 0, std::string::npos);
			break;
		}
		case Target:
			result.assign(FormatTargetValue(value),
				0, std::string::npos);
			break;
		case TextureUVs:
		{
			D3DXVECTOR4 input;
			memcpy(&input, value, sizeof(input));
			result.assign(FormatUVs(input, 5),
				0, std::string::npos);
			break;
		}
		case SnapBone:
		{
			const BYTE* input = value;
			result.assign(input ? reinterpret_cast<const char*>(input) : "");
			break;
		}
		case Integer: case TriggeredEffect:
			integerValue = *reinterpret_cast<const LONG*>(value);
		formatInteger:
			FormatString(result, "%d", integerValue);
			break;
		default:
			break;
		}
		return std::string(result); //4E1D80 copies before the local result dies.
	}

	VFX_ResourceData VFX_Parameter::GetTypeAndValue() const
	{
		VFX_ResourceData result;
		DWORD offset = 0;
		switch (type)
		{
		case Integer: case TriggeredEffect:
			FormatString(result.name, "%d", static_cast<int>(Read<LONG>(RawValue(), offset)));
			break;
		case Byte:
			FormatString(result.name, "%d", static_cast<int>(Read<BYTE>(RawValue(), offset)));
			break;
		case Float:
			result.name.assign(FormatFloat(Read<float>(RawValue(), offset), 3), 0, std::string::npos);
			break;
		case Boolean:
			FormatString(result.name, "%d", Read<BYTE>(RawValue(), offset) != 0);
			break;
		case Enumeration:
		{
			result.type.assign(GetEnumTypeName(subtype), 0, std::string::npos);
			//50085E destroys this temporary before reloading the value and subtype.
			const LONG input = Read<LONG>(RawValue(), offset);
			const LONG enumSubtype = subtype;
			result.name.assign(VFXEnumerationValueLabel(enumSubtype, input), 0, std::string::npos);
			break;
		}
		case String: case SnapBone:
		{
			const BYTE* input = RawValue();
			result.name = input ? reinterpret_cast<const char*>(input) : "";
			break;
		}
		case Vector:
		{
			D3DXVECTOR3 input;
			memcpy(&input, RawValue(), sizeof(input));
			result.name.assign(FormatVector3(input, 3), 0, std::string::npos);
			break;
		}
		case Rotation:
		{
			D3DXVECTOR3 input;
			memcpy(&input, RawValue(), sizeof(input));
			result.name.assign(FormatRotation(input, 3), 0, std::string::npos);
			break;
		}
		case Vector2:
		{
			D3DXVECTOR2 input;
			memcpy(&input, RawValue(), sizeof(input));
			result.name.assign(FormatVector2(input, 3), 0, std::string::npos);
			break;
		}
		case Color:
		{
			result.name.assign(FormatColor(*reinterpret_cast<const VFX_ColorBytes*>(RawValue())),
				0, std::string::npos);
			break;
		}
		case Resource:
		{
			const BYTE* input = RawValue();
			VFX_ResourceData resource(input);
			result = resource;
			break;
		}
		case Target:
			result.name.assign(FormatTargetValue(RawValue()), 0, std::string::npos);
			break;
		case TextureUVs:
		{
			D3DXVECTOR4 input;
			memcpy(&input, RawValue(), sizeof(input));
			result.name.assign(FormatUVs(input, 5), 0, std::string::npos);
			break;
		}
		default:
			break;
		}
		return VFX_ResourceData(result); //Copy both strings after all case temporaries die.
	}

	VFX_Target VFX_Parameter::GetTarget() const
	{
		DWORD offset = 0;
		VFX_Target result(0);
		switch (type)
		{
		case Integer: case Enumeration: case TriggeredEffect:
			result = static_cast<DWORD>(Read<LONG>(RawValue(), offset));
			break;
		case Target:
			memcpy(&result, RawValue(), sizeof(result));
			break;
		case Byte:
			result = Read<BYTE>(RawValue(), offset);
			break;
		case Boolean:
			result = Read<BYTE>(RawValue(), offset) != 0;
			break;
		case Float: case Vector: case Vector2: case Rotation: case TextureUVs:
			result = static_cast<DWORD>(static_cast<__int64>(Read<float>(RawValue(), offset)));
			break;
		case Color:
		{
			const BYTE* input = RawValue();
			result = input[0] | (input[1] << 8) | (input[2] << 16);
			break;
		}
		case String: case SnapBone:
			result = static_cast<DWORD>(static_cast<__int64>(atof(reinterpret_cast<const char*>(RawValue()))));
			break;
		case Resource:
		{
			const VFX_ResourceData input(RawValue());
			result = static_cast<DWORD>(static_cast<__int64>(atof(input.name.c_str())));
			break;
		}
		default:
			break;
		}
		return result;
	}

	std::string VFX_Parameter::GetResourceType() const
	{
		if (type != Resource)
			throw std::logic_error("VFX_Parameter resource conversion requires type10.");
		RequireValue(2);
		DWORD offset = 0;
		WORD size = Value<WORD>(*this);
		RequireValue(2 + size);
		return ReadStringValue(RawValue(), offset);
	}

	std::string VFX_Parameter::GetResourceName() const
	{
		if (type != Resource)
			throw std::logic_error("VFX_Parameter resource conversion requires type10.");
		RequireValue(2);
		DWORD offset = 0;
		WORD first = Value<WORD>(*this);
		RequireValue(2 + first + 2);
		ReadStringValue(RawValue(), offset);
		WORD second;
		memcpy(&second, RawValue() + offset, sizeof(second));
		RequireValue(offset + 2 + second);
		return ReadStringValue(RawValue(), offset);
	}

	void VFX_Parameter::GetVector(float* output, DWORD count) const
	{
		switch (count)
		{
		case 2:
		{
			const VFX_Vector2 result = GetVector2();
			memcpy(output, &result, sizeof(result));
			break;
		}
		case 3:
		{
			const VFX_Vector result = GetVector3();
			memcpy(output, &result, sizeof(result));
			break;
		}
		case 4:
		{
			const VFX_TextureCoordinates result = GetTextureUVs();
			memcpy(output, &result, sizeof(result));
			break;
		}
		default:
			throw std::logic_error("VFX_Parameter vector conversion requires 2, 3 or 4 values.");
		}
	}

	VFX_Vector VFX_Parameter::GetVector3() const
	{
		VFX_Vector result;
		DWORD offset = 0;
		switch (type)
		{
		case Integer: case Enumeration: case TriggeredEffect:
			result.xy.values[0] = static_cast<float>(Read<LONG>(RawValue(), offset));
			break;
		case Byte:
			result.xy.values[0] = Read<BYTE>(RawValue(), offset);
			break;
		case Boolean:
			result.xy.values[0] = Read<BYTE>(RawValue(), offset) != 0;
			break;
		case Float:
			memcpy(&result, RawValue(), sizeof(float));
			break;
		case String:
		{
			result = ParseVector3Value(reinterpret_cast<const char*>(RawValue()));
			break;
		}
		case Vector: case Rotation: case TextureUVs:
			memcpy(&result, RawValue(), sizeof(result));
			break;
		case Color:
		{
			const BYTE* input = RawValue();
			const float z = input[2], y = input[1], x = input[0];
			result = VFX_Vector(x, y, z);
			break;
		}
		case Resource:
		{
			const VFX_ResourceData input(RawValue());
			result = ParseVector3Value(input.name.c_str());
			break;
		}
		case Target:
			result.xy.values[0] = static_cast<float>(static_cast<LONG>(
				reinterpret_cast<const VFX_Target*>(RawValue())->Value()));
			break;
		case SnapBone:
		{
			result = ParseVector3Value(reinterpret_cast<const char*>(RawValue()));
			break;
		}
		default:
			break; //Native Vector2 case is absent.
		}
		return result;
	}

	VFX_Vector2 VFX_Parameter::GetVector2() const
	{
		VFX_Vector2 result;
		DWORD offset = 0;
		switch (type)
		{
		case Integer: case Enumeration: case TriggeredEffect:
			result.values[0] = static_cast<float>(Read<LONG>(RawValue(), offset));
			break;
		case Byte:
			result.values[0] = Read<BYTE>(RawValue(), offset);
			break;
		case Boolean:
			result.values[0] = Read<BYTE>(RawValue(), offset) != 0;
			break;
		case Float:
			memcpy(&result, RawValue(), sizeof(float));
			break;
		case String:
		{
			result = ParseVector2Value(reinterpret_cast<const char*>(RawValue()));
			break;
		}
		case Vector: case Vector2: case Rotation: case TextureUVs:
			memcpy(&result, RawValue(), sizeof(result));
			break;
		case Color:
		{
			const BYTE* input = RawValue();
			const float y = input[1], x = input[0];
			result = VFX_Vector2(x, y);
			break;
		}
		case Resource:
		{
			const VFX_ResourceData input(RawValue());
			result = ParseVector2Value(input.name.c_str());
			break;
		}
		case Target:
			result.values[0] = static_cast<float>(static_cast<LONG>(
				reinterpret_cast<const VFX_Target*>(RawValue())->Value()));
			break;
		case SnapBone:
		{
			result = ParseVector2Value(reinterpret_cast<const char*>(RawValue()));
			break;
		}
		default:
			break;
		}
		return result;
	}

	VFX_TextureCoordinates VFX_Parameter::GetTextureUVs() const
	{
		VFX_TextureCoordinates result;
		DWORD offset = 0;
		switch (type)
		{
		case Integer: case Enumeration: case TriggeredEffect:
			result.values[0] = static_cast<float>(Read<LONG>(RawValue(), offset));
			break;
		case Byte:
			result.values[0] = Read<BYTE>(RawValue(), offset);
			break;
		case Boolean:
			result.values[0] = Read<BYTE>(RawValue(), offset) != 0;
			break;
		case Float:
			memcpy(&result, RawValue(), sizeof(float));
			break;
		case String:
		{
			result = ParseTextureUVs(reinterpret_cast<const char*>(RawValue()));
			break;
		}
		case Vector:
			memcpy(&result, RawValue(), 3 * sizeof(float));
			break;
		case Vector2:
			memcpy(&result, RawValue(), 2 * sizeof(float));
			break;
		case Rotation:
		{
			const BYTE* input = RawValue();
			for (DWORD i = 0; i < 3; ++i)
			{
				float component;
				memcpy(&component, input + i * sizeof(float), sizeof(component));
				result.values[i] = component * 0.0027777778f;
			}
			break;
		}
		case Color:
		{
			const BYTE* input = RawValue();
			const float z = input[2] * 0.0039215689f;
			const float y = input[1] * 0.0039215689f;
			const float x = input[0] * 0.0039215689f;
			result = VFX_TextureCoordinates(x, y, z, 0.0f);
			break;
		}
		case Resource:
		{
			const VFX_ResourceData input(RawValue());
			result = ParseTextureUVs(input.name.c_str());
			break;
		}
		case Target:
			result.values[0] = static_cast<float>(static_cast<LONG>(
				reinterpret_cast<const VFX_Target*>(RawValue())->Value()));
			break;
		case TextureUVs:
			memcpy(&result, RawValue(), sizeof(result));
			break;
		case SnapBone:
		{
			result = ParseTextureUVs(reinterpret_cast<const char*>(RawValue()));
			break;
		}
		default:
			break;
		}
		return result;
	}

	VFX_Rotation VFX_Parameter::GetRotation() const
	{
		VFX_Rotation result;
		DWORD offset = 0;
		switch (type)
		{
		case Integer: case Enumeration: case TriggeredEffect:
			result.values[0] = static_cast<float>(Read<LONG>(RawValue(), offset));
			break;
		case Target:
			result.values[0] = static_cast<float>(static_cast<LONG>(
				reinterpret_cast<const VFX_Target*>(RawValue())->Value()));
			break;
		case Byte:
			result.values[0] = Read<BYTE>(RawValue(), offset);
			break;
		case Boolean:
			result.values[0] = Read<BYTE>(RawValue(), offset) != 0;
			break;
		case Float:
			memcpy(result.values, RawValue(), sizeof(float));
			break;
		case String: case SnapBone:
			result = ParseRotation(reinterpret_cast<const char*>(RawValue()));
			break;
		case Vector: case Rotation:
			memcpy(result.values, RawValue(), sizeof(result.values));
			break;
		case Vector2:
			memcpy(result.values, RawValue(), 2 * sizeof(float));
			break;
		case Color:
		{
			const BYTE* input = RawValue();
			const float z = input[2], y = input[1], x = input[0];
			result = VFX_Rotation(x, y, z);
			break;
		}
		case Resource:
		{
			const VFX_ResourceData input(RawValue());
			result = ParseRotation(input.name.c_str());
			break;
		}
		case TextureUVs:
		{
			const BYTE* input = RawValue();
			float components[3];
			memcpy(components, input, sizeof(components));
			const float z = components[2] * 360.0f;
			const float y = components[1] * 360.0f;
			const float x = components[0] * 360.0f;
			result = VFX_Rotation(x, y, z);
			break;
		}
		default:
			break;
		}
		return result;
	}

	VFX_ColorBytes VFX_Parameter::GetColor() const
	{
		VFX_ColorBytes result;
		DWORD offset = 0;
		switch (type)
		{
		case Integer: case Enumeration: case Color:
			memcpy(result.values, RawValue(), sizeof(result));
			break;
		case Byte: case Boolean: case TriggeredEffect:
			result.values[0] = Read<BYTE>(RawValue(), offset);
			break;
		case Float:
		{
			const float* input = reinterpret_cast<const float*>(RawValue());
			result.values[0] = static_cast<BYTE>(static_cast<DWORD>(static_cast<__int64>(*input)));
			result.values[1] = static_cast<BYTE>(static_cast<DWORD>(static_cast<__int64>(*input)) >> 8);
			result.values[2] = static_cast<BYTE>(static_cast<DWORD>(static_cast<__int64>(*input)) >> 16);
			break;
		}
		case String:
		{
			result = ParseColor(reinterpret_cast<const char*>(RawValue()));
			break;
		}
		case Vector: case Rotation:
		{
			const BYTE* input = RawValue();
			for (DWORD i = 0; i < 3; ++i)
			{
				float component;
				memcpy(&component, input + i * sizeof(float), sizeof(component));
				const __int64 integer = static_cast<__int64>(component);
				result.values[i] = static_cast<BYTE>(integer);
			}
			break;
		}
		case Vector2:
		{
			const BYTE* input = RawValue();
			for (DWORD i = 0; i < 2; ++i)
			{
				float component;
				memcpy(&component, input + i * sizeof(float), sizeof(component));
				const __int64 integer = static_cast<__int64>(component);
				result.values[i] = static_cast<BYTE>(integer);
			}
			break;
		}
		case TextureUVs:
		{
			const BYTE* input = RawValue();
			for (DWORD i = 0; i < 3; ++i)
			{
				float component;
				memcpy(&component, input + i * sizeof(float), sizeof(component));
				//4FEC92 feeds the wide product straight to FTOL; no float spill.
				const __int64 integer = static_cast<__int64>(double(component) * double(255.0f));
				result.values[i] = static_cast<BYTE>(integer);
			}
			break;
		}
		case Resource:
		{
			const VFX_ResourceData input(RawValue());
			result = ParseColor(input.name.c_str());
			break;
		}
		case Target:
			//Native reloads the payload before each real opaque selector getter.
			result.values[0] = static_cast<BYTE>(reinterpret_cast<const VFX_Target*>(RawValue())->Value());
			result.values[1] = static_cast<BYTE>(reinterpret_cast<const VFX_Target*>(RawValue())->Value() >> 8);
			result.values[2] = static_cast<BYTE>(reinterpret_cast<const VFX_Target*>(RawValue())->Value() >> 16);
			break;
		case SnapBone:
		{
			result = ParseColor(reinterpret_cast<const char*>(RawValue()));
			break;
		}
		default:
			break;
		}
		return result;
	}

	LONG VFX_Parameter::GetTypeID(const char* input)
	{
		char token[1024];
		Tokenize(token, input, 1024, " \t\n\r,;", 0, 1);
		for (LONG i = 0; i < 15; ++i)
			if (!_mbscmp(reinterpret_cast<const unsigned char*>(parameterTypes[i]),
				reinterpret_cast<const unsigned char*>(token))) return i;
		return -1;
	}

	LONG VFX_Parameter::GetSubtypeID(const char* input)
	{
		char token[1024];
		const char* suffix = Tokenize(token, input, 1024, " \t\n\r,;", 0, 1);
		if (suffix != NULL && strlen(suffix) != 0)
		{
			Tokenize(token, suffix, 1024, " \t\n\r,;", 0, 1);
			return GetEnumTypeID(token);
		}
		return -1;
	}

	LONG VFX_Parameter::GetEnumTypeID(const char* input)
	{
		return FindString(input, enumTypes, 15);
	}

	void VFX_Parameter::SetText(const char* input, LONG requestedType, LONG enumSubtype)
	{
		type = requestedType;
		switch (type)
		{
		case Integer:
		{
			LONG parsed = input ? atol(input) : 0;
			SetValue(&parsed, 4);
			break;
		}
		case Byte:
		{
			BYTE parsed = input ? static_cast<BYTE>(atol(input)) : 0;
			SetValue(&parsed, 1);
			break;
		}
		case Float:
		{
			float parsed = input ? static_cast<float>(atof(input)) : 0.0f;
			SetValue(&parsed, 4);
			break;
		}
		case Boolean:
		{
			BYTE parsed;
			if (input)
			{
				if (!_mbsicmp(reinterpret_cast<const unsigned char*>(input),
					reinterpret_cast<const unsigned char*>("true"))) parsed = 1;
				else parsed = atol(input) != 0;
			}
			else parsed = 0;
			SetValue(&parsed, 1);
			break;
		}
		case Enumeration:
		{
			LONG parsed = FindVFXEnumerationValue(enumSubtype, input);
			SetValue(&parsed, 4);
			break;
		}
		case String:
			SetValue(input, input ? static_cast<DWORD>(strlen(input)) + 1 : 0);
			break;
		case Vector:
		{
			const VFX_Vector parsed = ParseVector3Value(input);
			SetValue(&parsed, 12);
			break;
		}
		case Vector2:
		{
			const VFX_Vector2 parsed = ParseVector2Value(input);
			SetValue(&parsed, 8);
			break;
		}
		case Rotation:
		{
			const VFX_Rotation parsed = ParseRotation(input);
			SetValue(&parsed, 12);
			break;
		}
		case Color:
		{
			const VFX_ColorBytes parsed = ParseColor(input);
			SetValue(&parsed, 3);
			break;
		}
		case Resource:
		{
			const VFX_ResourceData parsed = ParseResourceValue(input);
			DWORD count = 0;
			parsed.Write(NULL, count);
			if (static_cast<LONG>(count) > 0)
			{
				BYTE* bytes = new BYTE[count];
				if (bytes)
				{
					DWORD cursor = 0;
					parsed.Write(bytes, cursor);
					SetValue(bytes, count);
					delete[] bytes;
				}
			}
			break;
		}
		case Target:
		{
			const VFX_Target parsed = ParseTargetValue(input);
			SetValue(&parsed, 4);
			break;
		}
		case TextureUVs:
		{
			const VFX_TextureCoordinates parsed = ParseTextureUVs(input);
			SetValue(&parsed, 16);
			break;
		}
		case SnapBone:
			SetValue(input, input ? static_cast<DWORD>(strlen(input)) + 1 : 0);
			break;
		case TriggeredEffect:
		{
			LONG parsed = input ? atol(input) : -1;
			SetValue(&parsed, 4);
			break;
		}
		default:
			type = Unknown;
			break;
		}
	}

	VFX_ParameterHolder::VFX_ParameterHolder()
	{
	}

	VFX_ParameterHolder::VFX_ParameterHolder(const VFX_ParameterHolder& input)
	{
		*this = input;
	}

	VFX_ParameterHolder::~VFX_ParameterHolder()
	{
		//4FAC40: natural reverse member lifetime destroys order before parameters.
	}

	VFX_ParameterHolder& VFX_ParameterHolder::operator=(const VFX_ParameterHolder& input)
	{
		//4FB000 clears even on self-assignment, then captures the source's live order.
		Clear();
		for (ParameterOrder::const_iterator i = input.order.begin(); i != input.order.end(); ++i)
			Insert(**i, 1);
		return *this;
	}

	VFX_Parameter* VFX_ParameterHolder::GetParameter(LONG index)
	{
		if (index < 0 || index >= static_cast<LONG>(order.size())) return NULL;
		return order[index];
	}

	VFX_Parameter* VFX_ParameterHolder::Find(const char* name)
	{
		if (!name) return NULL;
		const std::string key(name);
		ParameterMap::iterator found = parameters.find(key);
		if (found == parameters.end()) return NULL;
		return &found->second;
	}

	const VFX_Parameter* VFX_ParameterHolder::Find(const char* name) const
	{
		if (!name) return NULL;
		const std::string key(name);
		ParameterMap::const_iterator found = parameters.find(key);
		if (found == parameters.end()) return NULL;
		return &found->second;
	}

	BYTE VFX_ParameterHolder::Insert(const VFX_Parameter& input, BYTE replace)
	{
		const std::string key(input.name);
		ParameterMap::iterator found = parameters.find(key);
		if (found == parameters.end())
		{
			parameters[key] = input;
			order.push_back(&parameters[key]);
			return 1;
		}
		if (replace)
		{
			found->second = input;
			return 1;
		}
		return 0;
	}

	BYTE VFX_ParameterHolder::Read(const BYTE* buffer, DWORD& offset)
	{
		Clear();
		LONG count;
		ReadVfxBuffer(&count, 4, 1, buffer, offset);
		for (LONG i = 0; i < count; ++i)
		{
			VFX_Parameter input;
			if (!input.ReadParameter(buffer, offset)) return 0;
			Insert(input, 1);
		}
		return 1;
	}

	BYTE VFX_ParameterHolder::Write(BYTE* buffer, DWORD& offset) const
	{
		LONG count = static_cast<LONG>(order.size());
		WriteVfxBuffer(&count, 4, 1, buffer, offset);
		for (ParameterOrder::const_iterator i = order.begin(); i != order.end(); ++i)
		{
			if (!(*i)->WriteParameter(buffer, offset)) return 0;
		}
		return 1;
	}
}
