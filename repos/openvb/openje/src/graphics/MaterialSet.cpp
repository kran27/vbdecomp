#include "MeshData.hpp"

Gfx_MaterialSet::Gfx_MaterialSet(const Gfx_MaterialSet& source)
{
	count = 0;
	materials = 0;
	*this = source;
	// These pointers are deliberately not published before material assignment.
	groupCounts = new DWORD[6];
	groups = new std::list<DWORD>[6];
	ResetGroups();
}

Gfx_MaterialSet::Gfx_MaterialSet()
	: count(0), groupCounts(new DWORD[6]), groups(new std::list<DWORD>[6]), materials(0)
{
	ResetGroups();
}

Gfx_MaterialSet::~Gfx_MaterialSet()
{
	count = 0;
	ResetGroups();
	if (materials) { delete[] materials; materials = 0; }
	if (groupCounts) { delete[] groupCounts; groupCounts = 0; }
	if (groups) { delete[] groups; groups = 0; }
}

void Gfx_MaterialSet::Clear()
{
	count = 0;
	ResetGroups();
	if (materials) { delete[] materials; materials = 0; }
}

void Gfx_MaterialSet::ResetGroups()
{
	for (int i = 0; i < 6; ++i)
	{
		groups[i].clear();
		groupCounts[i] = 0;
	}
}

void Gfx_MaterialSet::SetCount(DWORD size)
{
	count = size;
	if (materials) { delete[] materials; materials = 0; }
	materials = new Gfx_Material[size];
}

void Gfx_MaterialSet::RebuildGroups()
{
	ResetGroups();
	for (DWORD i = 0; i < count; ++i)
	{
		DWORD group = materials[i].mode28;
		groups[group].push_back(i);
		++groupCounts[group];
	}
}

void Gfx_MaterialSet::ResetValues()
{
	for (DWORD i = 0; i < count; ++i)
	{
		Gfx_Material& material = materials[i];
		material.style2 = material.style1;
		material.mode28 = material.mode24;
		material.ResetTextureFactor();
	}
}
