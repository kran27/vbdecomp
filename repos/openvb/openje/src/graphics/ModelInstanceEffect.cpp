#include "ModelInstance.hpp"
#include "ModelMeshPacket.hpp"
#include "SkinMeshPacket.hpp"

void Gfx_ModelInstance::ResetMaterialValues()
{
	Gfx_ModelMeshPkt* mesh = firstMesh;
	if (mesh && mesh->overrideMaterials)
	{
		mesh->overrideMaterials->ResetValues();
		mesh->overrideMaterials->RebuildGroups();
	}
	if (firstSkin) firstSkin->ResetMaterialValues();
}

void Gfx_ModelInstance::SetMaterialColors(DWORD input, BYTE useRGB, BYTE useAlpha,
	const DWORD* emissive)
{
	const BYTE mergedRGB = flagC8 || useRGB;
	flagC8 = BYTE(mergedRGB);
	const BYTE mergedAlpha = flagC9 || useAlpha;
	flagC9 = BYTE(mergedAlpha);
	BYTE red = BYTE(input >> 16);
	BYTE green = BYTE(input >> 8);
	BYTE blue = BYTE(input);
	BYTE alpha;
	if (useAlpha) alpha = BYTE(input >> 24);
	else alpha = reinterpret_cast<const BYTE*>(&color)[3];
	if (!useRGB)
	{
		red = reinterpret_cast<const BYTE*>(&color)[2];
		green = reinterpret_cast<const BYTE*>(&color)[1];
		blue = reinterpret_cast<const BYTE*>(&color)[0];
	}
	Gfx_ModelMeshPkt* mesh = firstMesh;
	const DWORD merged = D3DCOLOR_RGBA(red, green, blue, alpha);
	color = merged;
	if (mesh) mesh->SetMaterialColors(BYTE(mergedRGB), BYTE(mergedAlpha), merged, emissive);
	Gfx_SkinMeshPkt* skin = firstSkin;
	if (skin) skin->SetMaterialColors(flagC8, flagC9, color, emissive);
}

void Gfx_ModelInstance::SetRenderPhase(DWORD phase)
{
	if (firstMesh) firstMesh->SetRenderPhase(phase);
	if (firstSkin) firstSkin->SetRenderPhase(phase);
}
