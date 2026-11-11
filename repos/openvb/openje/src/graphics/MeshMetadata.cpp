#include "MeshData.hpp"
#include "D3DState.hpp"
#include "../diagnostics/DebugOutput.hpp"

// Original VA0x4AB680. The actual D3D global's production lifetime is pending;
// this member retains an unresolved external owner, never a fallback limit.
void Gfx_MeshData::SetVertexDesc(DWORD fvf, DWORD stride, DWORD count)
{
	vertexFVF = fvf;
	vertexStride = stride;
	vertexCount = count;
	DWORD maximum = g_D3D.vertexLimit;
	if (count > maximum)
		JE::LogDebugString("Gfx_MeshData::SetVertexDesc - invalid vertex count: %d (max = %d)\n", count, maximum);
}
