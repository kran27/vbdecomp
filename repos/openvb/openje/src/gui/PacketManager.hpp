#pragma once

#include <windows.h>
#include <list>

class Gfx_MeshInstance;
class Gfx_Viewport;
class Gfx_UIPacketBuffer;

namespace GUI
{
	// Original RTTI .?AVCPktManager@GUI@@, table0x66F7B4. These are borrowed
	// mesh-instance packets, not copies of the historical preview UIMesh type.
	class CPktManager
	{
	public:
		CPktManager(); // 0x4B6200; constructs all five real lists, then clears them.
		virtual ~CPktManager(); // 0x4B6340.
		void Clear(); // 0x4B6450; borrowed payloads survive, optimization unchanged.
		void Submit(Gfx_MeshInstance* instance, Gfx_Viewport* target, DWORD kind); // 0x4B6750.
		void Flush(); // 0x4B7140; recovered batching; full backend/GUI lifetime pending.
		void BatchQuads(std::list<Gfx_MeshInstance*>::iterator& position, LONG count, DWORD textureHandle); // 0x4B6F90.
		void BatchTriangles(std::list<Gfx_MeshInstance*>::iterator& position, LONG count, LONG primitives,
			DWORD textureHandle, DWORD color); // 0x4B7050.

		DWORD optimizationLevel; // +04; original constructor writes1.
		std::list<Gfx_MeshInstance*> packets0; // +08; grouped by first texture handle.
		std::list<Gfx_MeshInstance*> packets1; // +14; append order.
		std::list<Gfx_MeshInstance*> packets2; // +20; append order.
		std::list<Gfx_MeshInstance*> packets3; // +2C; texture plus packed diffuse color.
		std::list<Gfx_UIPacketBuffer*> entries38; // +38; borrowed batch bases, storage owns their lifetime.
	};
}
