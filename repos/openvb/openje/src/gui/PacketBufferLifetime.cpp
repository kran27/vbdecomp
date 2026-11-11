#include "PacketBuffer.hpp"

Gfx_UIPacketBuffer::Gfx_UIPacketBuffer()
{
	// The original first constructs mesh and instance. Body writes follow both;
	// putting these four initializations in an initializer list changes that order.
	primitive = 0;
	vertices = 0;
	material = 0;
	indices = 0;
}

Gfx_UIPacketBuffer::~Gfx_UIPacketBuffer()
{
	// Normal reverse members: instance, then mesh (Free and materialSet).
	// vertices and indices are borrowed; there is no separate arena free here.
}

