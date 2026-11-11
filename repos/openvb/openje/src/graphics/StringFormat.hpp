#pragma once

#include <string>

struct D3DXVECTOR2;
struct D3DXVECTOR3;
struct D3DXVECTOR4;

namespace JE
{
	struct VFX_ColorBytes;
	VFX_ColorBytes __cdecl ParseColor(const char* input); //52E920, ownedRGB result.
	std::string __cdecl FormatColor(VFX_ColorBytes input); //52E860, packedRGB byvalue.
	struct VFX_ResourceData;
	struct VFX_Target;
	VFX_ResourceData __cdecl ParseResourceValue(const char* input); //52E5B0, owned56B result.
	VFX_Target __cdecl ParseTargetValue(const char* input); //502170, owned4B result.
	unsigned char __cdecl IsStringBlank(const char* input, const char* delimiters); //4A1170, AL result.
	char* __cdecl TrimString(char* output, const char* input, const char* delimiters); //4A10A0.
	struct VFX_Rotation;
	struct VFX_Vector;
	struct VFX_Vector2;
	struct VFX_TextureCoordinates;
	VFX_Vector __cdecl ParseVector3Value(const char* input); //523D00, owned12B result.
	VFX_Vector2 __cdecl ParseVector2Value(const char* input); //52F8F0, owned8B result.
	VFX_TextureCoordinates __cdecl ParseTextureUVs(const char* input); //52ECE0, owned16B result.
	VFX_Rotation __cdecl ParseRotation(const char* input); //52C290, owned12B result.
	const char* __cdecl Tokenize(char* output, const char* input, int capacity,
		const char* delimiters, unsigned char multibyte, unsigned char copyLeading); //4A1550.
	std::string& __cdecl UpperString(std::string& value); //4A15A0, owned C-string buffer policy.
	void __cdecl FormatString(std::string& output, const char* format, ...); //4A1A30.
	std::string __cdecl FormatFloat(float input, int precision); //4A1A40, fresh value result.
	// SDK values supply the observed by-value word transport; native class names
	// are unresolved. Scalar overloads are convenience forwards, without credit.
	std::string __cdecl FormatVector2(D3DXVECTOR2 input, int precision); //52F7A0.
	std::string __cdecl FormatVector3(D3DXVECTOR3 input, int precision); //523B50.
	std::string __cdecl FormatUVs(D3DXVECTOR4 input, int precision); //52EAC0.
	std::string __cdecl FormatRotation(D3DXVECTOR3 input, int precision); //52C2F0.
	std::string __cdecl FormatVector2(float x, float y, int precision);
	std::string __cdecl FormatVector3(float x, float y, float z, int precision);
	std::string __cdecl FormatUVs(float x, float y, float z, float w, int precision);
	std::string __cdecl FormatRotation(float x, float y, float z, int precision);
}
