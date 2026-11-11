#include "ModelInstance.hpp"
#include "ModelMeshPacket.hpp"
#include "SkinMeshPacket.hpp"

void Gfx_ModelInstance::SetAmbientOverride(BYTE value)
{
	if (firstMesh && firstMesh->packet) firstMesh->packet->ambientOverride=value;
	if (firstSkin) firstSkin->ambientOverride=value;
}
void Gfx_ModelInstance::SetAmbientColor(DWORD value)
{
	if (firstMesh && firstMesh->packet) firstMesh->packet->ambientColor=value;
	if (firstSkin) firstSkin->ambientColor=value;
}
