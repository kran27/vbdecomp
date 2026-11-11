#include "OctreeRender.hpp"
#include "diagnostics/DebugOutput.hpp"

void Octree2Gfx::ResetLights()
{
	for (DWORD index=0;index<meshCount;++index) packets[index]->ClearLights();
}

void Octree2Gfx::AddLight(Gfx_Light* light,const std::map<DWORD,std::list<DWORD> >* nodes)
{
	if (!light) return;
	if (nodes->empty()) return;
	// The original does not guard a NULL node map for a non-NULL light.
	for (std::map<DWORD,std::list<DWORD> >::const_iterator mesh=nodes->begin();
		mesh!=nodes->end();++mesh)
	{
		const DWORD meshID=mesh->first;
		if (meshID>=meshCount)
		{
			JE::LogDebugString("Octree2Gfx::AddLight - bad Mesh ID: %d\n",meshID);
			continue;
		}
		for (std::list<DWORD>::const_iterator node=mesh->second.begin();node!=mesh->second.end();++node)
		{
			if (*node<nodeCount)
				packets[meshID]->AddLight(light,*node);
			else JE::LogDebugString("Octree2Gfx::AddLight - invalid NodeID: %x\n",*node);
		}
	}
}
