#include "TextureTransform.hpp"
#include "D3DState.hpp"
#include <cstring>
#include <math.h>

namespace
{
	int Integral(double value)
	{
		// Original __ftol2 retains only the signed low DWORD of its64-bit result.
		return static_cast<int>(static_cast<__int64>(value));
	}
	double Fraction(double value) { return value - Integral(value); }
}

HRESULT __stdcall Gfx_SetTextureTransform(DWORD stage, BYTE flags,
	const Gfx_TextureParameters* parameters, float seconds)
{
	D3DXMATRIX matrix = g_D3D.identity;
	if (flags & 1)
	{
		float speed;
		std::memcpy(&speed, &parameters->value00, sizeof(speed));
		double product = static_cast<double>(seconds) * speed;
		// Original __ftol2 consumes a 64-bit conversion but keeps signed EAX.
		int integral = static_cast<int>(static_cast<__int64>(product));
		matrix._31 = static_cast<float>(product - integral + matrix._31);
	}
	if (flags & 2)
	{
		float speed;
		std::memcpy(&speed, &parameters->value04, sizeof(speed));
		double product = static_cast<double>(speed) * seconds;
		int integral = static_cast<int>(static_cast<__int64>(product));
		matrix._32 = static_cast<float>(product - integral + matrix._32);
	}
	if (flags & 4)
	{
		double phase = Fraction(double(parameters->value0C) * seconds) * double(6.2831855f);
		matrix._31 = float(sin(phase) * parameters->value08 + matrix._31);
	}
	if (flags & 8)
	{
		double phase = Fraction(double(parameters->value14) * seconds) * double(6.2831855f);
		matrix._32 = float(sin(phase) * parameters->value10 + matrix._32);
	}
	if (flags & 0x10)
	{
		double phase = Fraction(double(parameters->value20) * seconds) * double(6.2831855f);
		double cosine = cos(phase);
		float sine = float(sin(phase)); // Original stores sine before matrix/center arithmetic.
		float centerU = parameters->value18, centerV = parameters->value1C;
		matrix._12 = sine;
		matrix._11 = float(cosine);
		matrix._21 = -sine;
		matrix._22 = float(cosine);
		matrix._31 = float(centerU * (1.0 - cosine) - double(matrix._21) * centerV + matrix._31);
		matrix._32 = float((1.0 - cosine) * centerV - double(centerU) * sine + matrix._32);
	}
	if (flags & 0x20)
	{
		// Actual signed IDIV uses truncated frame/count, then an integer row.
		// A zero frame count remains the original invalid division domain.
		int frame = Integral(double(parameters->value24) * seconds);
		int frames = Integral(parameters->value30);
		int selected = frame % frames;
		double column = double(selected) * parameters->value34;
		int row = Integral(column);
		matrix._31 = float(column - row + matrix._31);
		matrix._32 = float(double(parameters->value38) * row + matrix._32);
	}
	if (flags & 0x3F)
	{
		g_D3D.SetTransform(static_cast<D3DTRANSFORMSTATETYPE>(stage + 16), &matrix);
		return g_D3D.SetTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, 2);
	}
	return g_D3D.SetTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
}
