#pragma once

#include "configuration/ConfigurationTypes.hpp"
#include <windows.h>

class Gfx_System_Interface;
class Octree2Gfx;
namespace JE
{
	void ReadShadowConfiguration(DWORD& mode, const Configuration::Settings& settings); //581C30 subset.
	void InitializeProjectedShadowTexture(Gfx_System_Interface& system); //581A92..581B36.
	void SetGraphicsShadowMode(DWORD& mode, Octree2Gfx* floor,
		Gfx_System_Interface& system, DWORD requested); //580F80.
	DWORD CycleGraphicsShadowMode(DWORD& mode, Octree2Gfx* floor,
		Gfx_System_Interface& system); //580FF0.
}
