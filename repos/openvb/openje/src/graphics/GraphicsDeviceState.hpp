#pragma once

#include <windows.h>

class Gfx_System_Interface;
class Gfx_Viewport;

namespace JE
{
	HRESULT SetGraphicsAlphaTest(BYTE& flag, BYTE enabled); //5BDE20, flag stored last.
	void SetGraphicsClearColor(Gfx_Viewport& viewport, BYTE red, BYTE green, BYTE blue); //5C0860.
	HRESULT SetGraphicsWireframe(BYTE enabled); //5C03E0, nonzero selects WIREFRAME.
}
