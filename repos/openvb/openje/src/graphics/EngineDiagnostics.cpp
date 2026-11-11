#include "Engine.hpp"
#include "D3DState.hpp"
#include "BufferManager.hpp"
#include "TextureManager.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

void Gfx_Engine::GetVRAMStatistics(DWORD* output)
{
	std::memset(output, 0, 68);
	output[1] = bufferManager->vertexHighWater;
	output[2] = bufferManager->indexHighWater;
	output[3] = bufferManager->vertexBytes;
	output[4] = bufferManager->indexBytes;
	DWORD renderTargets = 0;
	D3DSURFACE_DESC descriptor;
	if (automapRenderTarget)
	{
		automapRenderTarget->GetDesc(&descriptor);
		renderTargets = descriptor.Size;
	}
	if (automapDepthSurface)
	{
		automapDepthSurface->GetDesc(&descriptor);
		renderTargets += descriptor.Size;
	}
	if (fogOfWarTarget)
	{
		fogOfWarTarget->GetLevelDesc(0, &descriptor);
		renderTargets += descriptor.Size;
	}
	if (projectedShadowTarget)
	{
		projectedShadowTarget->GetLevelDesc(0, &descriptor);
		renderTargets += descriptor.Size;
	}
	DWORD depth, stencil;
	BYTE lockable;
	g_D3D.DescribeTextureFormat(g_D3D.presentation.BackBufferFormat, &depth, NULL);
	renderTargets += 2 * g_D3D.presentation.BackBufferWidth * g_D3D.presentation.BackBufferHeight * (depth >> 3);
	g_D3D.DescribeDepthStencilFormat(g_D3D.presentation.AutoDepthStencilFormat, &depth, &stencil, &lockable);
	renderTargets += g_D3D.presentation.BackBufferWidth * g_D3D.presentation.BackBufferHeight * ((depth + stencil) >> 3);
	output[9] = renderTargets + textureManager->defaultFirstMipSize;
	output[10] = renderTargets + textureManager->defaultTotalSize;
	output[11] = textureManager->managedFirstMipSize;
	output[12] = textureManager->managedTotalSize;
	output[13] = textureManager->systemFirstMipSize;
	output[14] = textureManager->systemTotalSize;
	output[15] = output[13] + output[9] + output[11];
	const DWORD total = textureManager->totalSize + renderTargets;
	output[16] = total;
	JE::LogDebugString("\nVRAM usage (est.)\n===================\n"
		"Current Geometry Data: %d (Bytes)\n"
		"\tVertex: %d Peak: %d (Bytes)\n"
		"\tIndex: %d Peak: %d (Bytes)\n"
		"CurrentTexture Data: Base: %d Total: %d (Bytes)\n"
		"\tDefault: Base: %d Total: %d (Bytes)\n"
		"\tManaged: Base: %d Total: %d (Bytes)\n"
		"\tSystem: Base: %d Total: %d (Bytes)\n\n",
		output[3] + output[4], output[3], output[1], output[4], output[2],
		output[15], total, output[9], output[10], output[11], output[12], output[13], output[14]);
}
