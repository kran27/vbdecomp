#pragma once

#include <d3d8.h>
#include <string>

class Gfx_Primitive
{
public:
	Gfx_Primitive(); // 0x41F190.
	~Gfx_Primitive(); // Compiler-owned string cleanup at 0x52AFB0; real std::string.
	DWORD value00;
	std::string name; // +04..1F under VC7.
	D3DPRIMITIVETYPE type; // +20.
	DWORD value24, value28, value2C, value30;
};
