#include "LineSystem.hpp"
#include "ModelNode.hpp"
#include "SystemInterface.hpp"

void Gfx_LineSystem::Draw(Gfx_Viewport* viewport) //50F4C0.
{
	if (owner)
	{
		D3DXMATRIX snapshot=static_cast<Gfx_ModelNode*>(owner)->worldTransform;
		transform=snapshot;
	}
	Gfx_PacketData* currentPacket=packet;
	if (currentPacket) { currentPacket->transformEnabled=1; currentPacket->transform=&transform; }
	if (enabled)
	{
		BuildGeometry();
		enabled=0;
	}
	if (static_cast<LONG>(vertexCount)>0)
		g_GfxSystem->QueueMeshInstances(instances,viewport,instanceCount);
}
