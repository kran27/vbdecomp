#pragma once

#include <windows.h>
#include <stddef.h>
#include <map>
#include <string>
#include <vector>
#include "configuration/ConfigurationTypes.hpp"
#include "VfxValueOwners.hpp"

namespace JE
{
	LONG FindVFXTargetLabel(const std::string& input); //502130; unknown label returns0.
	std::string VFXTargetLabelValue(LONG value); //502310; owned label or empty.
	LONG FindVFXEnumerationValue(LONG subtype, const char* input); //52FAB0; invalid returns-1.
	std::string VFXEnumerationValueLabel(LONG subtype, LONG value); //52FBA0; owned label or empty.

	//0x4B8130: shared binary-output span/count policy; NULL output counts only.
	DWORD WriteVfxBuffer(const void* input, DWORD size, DWORD count, BYTE* buffer, DWORD& offset);
	DWORD ReadVfxBuffer(void* output, DWORD size, DWORD count, const BYTE* buffer, DWORD& offset); //4B80E0.
	void WriteVfxString(const std::string& input, BYTE* buffer, DWORD& offset); //4B8340, cdecl.
	void ReadVfxString(std::string& output, const BYTE* buffer, DWORD& offset); //4B8480, output-ref API.

	// Original two-string value: type/name for resources, type/value for500760.
	struct VFX_ResourceData
	{
		VFX_ResourceData(); //40A220, two default string members.
		explicit VFX_ResourceData(const BYTE* input); //52E470, two counted strings.
		std::string type, name;
		BYTE Read(const BYTE* buffer, DWORD& offset); //52E410, both counted strings.
		BYTE Write(BYTE* buffer, DWORD& offset) const; //0x52E440.
	};

	// Proprietary VEG parameter data, using ordinary owning C++ objects. These
	// objects are not overlays of the original VC7 objects.
	class VFX_Parameter
	{
	public:
		enum Type
		{
			Unknown = -1, Integer = 0, Byte = 1, Float = 2, Boolean = 3,
			Enumeration = 4, String = 5, Vector = 6, Vector2 = 7,
			Rotation = 8, Color = 9, Resource = 10, Target = 11,
			TextureUVs = 12, SnapBone = 13, TriggeredEffect = 14
		};

		VFX_Parameter(); // 0x4FE1E0.
		VFX_Parameter(const VFX_Parameter& input); //0x5006F0, initially cap0.
		virtual ~VFX_Parameter(); // 0x4FE210.
		VFX_Parameter& operator=(const VFX_Parameter& input); //0x4FFCC0.
		void SetValueLimit(LONG limit); //0x4FDEA0, preserves the old capped prefix.
		BYTE ReadParameter(const BYTE* buffer, DWORD& offset); // 0x500580/0x4FF580.
		BYTE WriteParameter(BYTE* buffer, DWORD& offset) const; //0x501000.
		void WriteValue(BYTE* buffer, DWORD& offset) const; //0x500BC0.
		LONG GetInteger() const; // 0x4FF970.
		BOOL GetBool() const; // 0x4FFB60.
		double GetFloat() const; // 0x4FE280, original x87 return.
		LONG GetEnum() const; // 0x4FFB50, integer conversion thunk.
		VFX_Target GetTarget() const; //4FED60, owned four-byte target result.
		std::string GetString() const; //4FFD10, per-type formatting; unknown returns empty.
		std::string ToString() const; //500150, unchecked scalar conversion and fresh copy.
		VFX_ResourceData GetTypeAndValue() const; //500760, full per-type pair conversion.
		std::string GetResourceType() const;
		std::string GetResourceName() const;
		void GetVector(float* output, DWORD count) const; //Unclaimed compatibility dispatcher.
		VFX_Vector GetVector3() const; //4FE450, owned12-byte value.
		VFX_Vector2 GetVector2() const; //4FE690, owned8-byte value.
		VFX_TextureCoordinates GetTextureUVs() const; //4FEEF0, owned16-byte value.
		VFX_Rotation GetRotation() const; //0x4FE880, owned angular value.
		VFX_ColorBytes GetColor() const; //4FEB20, owned3-byte value.
		VFX_Parameter& SetInteger(LONG input); //4FDFB0.
		VFX_Parameter& SetByte(BYTE input); //4FDFD0.
		VFX_Parameter& SetBool(bool input); //4FE010.
		VFX_Parameter& SetFloat(float input); // 0x4FDFF0.
		VFX_Parameter& SetVector3(const VFX_Vector& input); //4FE040, whole12-byte value.
		VFX_Parameter& SetVector2(const float* input); //4FE060, raw8-byte input.
		VFX_Parameter& SetTextureUVs(const float* input); //4FE140, raw16-byte input.
		VFX_Parameter& SetRotation(const float* input); //4FE080, raw12-byte input.
		VFX_Parameter& SetColor(const BYTE* input); //4FE0A0, raw3-byte input.
		VFX_Parameter& SetTriggeredEffect(LONG input); //4FE160, raw signed32 value.
		VFX_Parameter& SetResource(const VFX_ResourceData& input); //4FE0C0.
		VFX_Parameter& SetString(const std::string& input); //4FE180.
		VFX_Parameter& SetSnapBone(const std::string& input); //4FE1B0.
		void SetText(const char* input, LONG type, LONG enumSubtype); // 0x4FF1B0.
		static LONG GetTypeID(const char* input); // 0x4FDCF0, first token.
		static LONG GetSubtypeID(const char* input); //0x4FDD90, second declaration token.
		static std::string GetTypeDescription(LONG type, LONG subtype); //0x4FFB70.
		static std::string GetEnumTypeName(LONG subtype); //52FB30, owned name or empty.
		static LONG GetEnumTypeID(const char* input); // 0x52FA70, complete name.
		const BYTE* RawValue() const;
		DWORD ValueSize() const;

		std::string name; // Original +4.
		LONG type, subtype; // Original +20/+24.
		LONG valueLimit; // Original +28: -1 unlimited, 0 disabled, positive cap.
		LONG valueSize; //Original +2C, signed allocation extent.
		BYTE* value; //Original +30, owned scalar-new storage released through CRT free.

	private:
		void ResizeValue(LONG count); //0x4FDE50.
		void SetValue(const void* input, LONG count); //0x4FDF50.
		void ReadValue(const BYTE* buffer, DWORD& offset); // 0x4FF580.
		void RequireValue(DWORD count) const;
		std::string ValueString() const;
	};

#if defined(_MSC_VER) && _MSC_VER == 1300
	typedef char VfxParameterOriginalSize[(sizeof(VFX_Parameter) == 0x34) ? 1 : -1];
	typedef char VfxParameterOriginalName[(offsetof(VFX_Parameter, name) == 0x04) ? 1 : -1];
	typedef char VfxParameterOriginalType[(offsetof(VFX_Parameter, type) == 0x20) ? 1 : -1];
	typedef char VfxParameterOriginalSubtype[(offsetof(VFX_Parameter, subtype) == 0x24) ? 1 : -1];
	typedef char VfxParameterOriginalLimit[(offsetof(VFX_Parameter, valueLimit) == 0x28) ? 1 : -1];
	typedef char VfxParameterOriginalExtent[(offsetof(VFX_Parameter, valueSize) == 0x2C) ? 1 : -1];
	typedef char VfxParameterOriginalStorage[(offsetof(VFX_Parameter, value) == 0x30) ? 1 : -1];
#endif

	class VFX_ParameterHolder
	{
	public:
		typedef std::map<std::string, VFX_Parameter, Configuration::CaseInsensitiveLess> ParameterMap;
		typedef std::vector<VFX_Parameter*> ParameterOrder;

		VFX_ParameterHolder(); // 0x4FAD40.
		VFX_ParameterHolder(const VFX_ParameterHolder& input);
		virtual ~VFX_ParameterHolder();
		VFX_ParameterHolder& operator=(const VFX_ParameterHolder& input);
		virtual BYTE Read(const BYTE* buffer, DWORD& offset); //0x4FAF20, slot4.
		virtual BYTE Write(BYTE* buffer, DWORD& offset) const; //0x4F9D40, slot8.
		VFX_Parameter* Find(const char* name); // 0x4FA3C0, case-insensitive.
		const VFX_Parameter* Find(const char* name) const;
		VFX_Parameter* GetParameter(LONG index); //4F9D10, signed live-order bounds.
		BYTE Insert(const VFX_Parameter& input, BYTE replace); // 0x4FADD0.
		void Clear(); // 0x4FA880.
		const ParameterOrder& GetParameterOrder() const { return order; }
		const ParameterMap& GetParameters() const { return parameters; }

	private:
		ParameterMap parameters;
		ParameterOrder order; // Original separate vector of stable map values.
	};
}
