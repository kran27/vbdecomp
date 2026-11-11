#pragma once

#include <d3dx8.h>

class Gfx_Texture;
namespace JE
{
	//Actual level-related fields and owned resources projected from Gfx_Engine.
	//This state is not an Engine allocation and does not replace its startup.
	class GraphicsLevelResources
	{
	public:
		explicit GraphicsLevelResources(BYTE& fogOfWarEnabled); //Corresponding5BFE10 defaults; sole Engine flag alias.
		~GraphicsLevelResources();
		void InitializeFormats(); //5BDCB0, real selected-device format/depth checks.
		void SetLevelAABB(const float* low, const float* high); //5C1600.
		void CreateFogOfWarRenderTarget(); //5BE300.
		void CreateProjectedShadowRenderTarget(); //5BEAC0.
		Gfx_Texture* AutomapMaskTexture() const; //5C0360, borrowed lookup.
		Gfx_Texture* VisibleMaskTexture() const; //5C0370, borrowed lookup.
		void Release(); //5C1BBB..5C1C4D owned subset, not full ShutdownDevice.

		D3DXVECTOR3 minimum, maximum; //Engine+74/+80.
		float width, height; //+8C/+90, X/Z extents.
		UINT maskWidth, maskHeight; //+94/+98.
		D3DFORMAT colorFormat, depthFormat; //+A8/+AC.
		DWORD automapMaskHandle, visibleMaskHandle; //+D4/+D8, owning TextureManager references.
		IDirect3DTexture8* fogOfWarTarget; //+DC, owning COM reference.
		UINT shadowWidth, shadowHeight; //+E0/+E4.
		IDirect3DTexture8* projectedShadowTarget; //+F0, owning COM reference.
		BYTE& fogOfWarEnabled; //Aliases sole EngineFrameState+891; creation failure clears it.
	private:
		GraphicsLevelResources(const GraphicsLevelResources&);
		GraphicsLevelResources& operator=(const GraphicsLevelResources&);
	};
}
