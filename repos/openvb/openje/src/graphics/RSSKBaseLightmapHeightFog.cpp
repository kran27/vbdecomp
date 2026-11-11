#include "RSSKBaseLightmapHeightFog.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "Material.hpp"
#include "HeightFog.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"
#include "Camera.hpp"
#include "TextureTransform.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

Gfx_RS_SK_B_LM_HeightFogState g_RSSKBaseLightmapHeightFogState = {0};

namespace
{
	// Exact original FX byte strings68A050/68AB90 are embedded below.
#define GFX_RS_SK_B_LM_HEIGHT_FOG_NORMAL_SOURCE \
	"DWORD\tBcop;\n" \
	"DWORD\tBaop;\n" \
	"DWORD\tLMcop;\n" \
	"DWORD\tLMaop;\n" \
	"DWORD\tTFcop;\n" \
	"DWORD\tTFaop;\n" \
	"DWORD\talphatest;\n" \
	"DWORD\tbLighting;\n" \
	"DWORD\tbFog;\n" \
	"DWORD\tDiffuseSource;\n" \
	"MATRIX\tfogmatrix;\n" \
	"Texture\tHFog;\n" \
	"Texture\tBase;\n" \
	"Texture\tLM;\n" \
	"Technique ffp_hfog\n" \
	"{\n" \
	"Pass p0\n" \
	"{\n" \
	"AlphaTestEnable\t\t\t= < alphatest >;\n" \
	"Lighting\t\t\t\t\t= < bLighting >;\n" \
	"FogEnable\t\t\t\t\t= < bFog >;\n" \
	"DiffuseMaterialSource\t\t= < DiffuseSource >;\n" \
	"AmbientMaterialSource\t\t= < DiffuseSource >;\n" \
	"Ambient\t\t\t\t\t= 0x00;\n" \
	"Texture[ 0 ]\t\t\t\t= < LM >;\n" \
	"TexCoordIndex[ 0 ]\t\t\t= 1;\n" \
	"ColorOp[ 0 ]\t\t\t\t= < LMcop >;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < LMaop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"Texture[ 1 ]\t\t\t\t= < Base >;\n" \
	"TexCoordIndex[ 1 ]\t\t\t= 0;\n" \
	"ColorOp[ 1 ]\t\t\t\t= < Bcop >;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
	"Texture[ 2 ]\t\t\t\t= < HFog >;\n" \
	"TextureTransform[ 2 ]\t\t= < fogmatrix >;\n" \
	"TexCoordIndex[ 2 ]\t\t\t= CameraSpacePosition | 2;\n" \
	"TextureTransformFlags[ 2 ]\t= Count2;\n" \
	"AddressU[ 2 ]\t\t\t\t= Clamp;\n" \
	"AddressV[ 2 ]\t\t\t\t= Clamp;\n" \
	"Mipfilter[ 2 ]\t\t\t\t= None;\n" \
	"ColorOp[ 2 ]\t\t\t\t= BlendTextureAlpha;\n" \
	"ColorArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 2 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= SelectArg2;\n" \
	"AlphaArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 2 ]\t\t\t\t= Current;\n" \
	"ColorOp[ 3 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 3 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"}\n" \
	"Technique ffp_hfog_2pass\n" \
	"{\n" \
	"Pass p0\n" \
	"{\n" \
	"AlphaTestEnable\t\t\t= < alphatest >;\n" \
	"Lighting\t\t\t\t\t= < bLighting >;\n" \
	"FogEnable\t\t\t\t\t= < bFog >;\n" \
	"DiffuseMaterialSource\t\t= < DiffuseSource >;\n" \
	"AmbientMaterialSource\t\t= < DiffuseSource >;\n" \
	"Ambient\t\t\t\t\t= 0x00;\n" \
	"Texture[ 0 ]\t\t\t\t= < LM >;\n" \
	"TexCoordIndex[ 0 ]\t\t\t= 1;\n" \
	"ColorOp[ 0 ]\t\t\t\t= < LMcop >;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < LMaop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"Texture[ 1 ]\t\t\t\t= < Base >;\n" \
	"TexCoordIndex[ 1 ]\t\t\t= 0;\n" \
	"ColorOp[ 1 ]\t\t\t\t= < Bcop >;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
	"ColorOp[ 2 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"Pass p1\n" \
	"{\n" \
	"AlphaBlendEnable\t\t\t= TRUE;\n" \
	"SrcBlend\t\t\t\t\t= SrcAlpha;\n" \
	"DestBlend\t\t\t\t\t= InvSrcAlpha;\n" \
	"BlendOp\t\t\t\t\t= Add;\n" \
	"Texture[ 0 ]\t\t\t\t= < Base >;\n" \
	"ColorOp[ 0 ]\t\t\t\t= SelectArg2;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"Texture[ 1 ]\t\t\t\t= < HFog >;\n" \
	"TextureTransform[ 1 ]\t\t= < fogmatrix >;\n" \
	"TexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\n" \
	"TextureTransformFlags[ 1 ]\t= Count2;\n" \
	"AddressU[ 1 ]\t\t\t\t= Clamp;\n" \
	"AddressV[ 1 ]\t\t\t\t= Clamp;\n" \
	"Mipfilter[ 1 ]\t\t\t\t= None;\n" \
	"ColorOp[ 1 ]\t\t\t\t= SelectArg1;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= Modulate;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
	"ColorOp[ 2 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"}\n"
	typedef char normalSourceLength[(sizeof(GFX_RS_SK_B_LM_HEIGHT_FOG_NORMAL_SOURCE) - 1 == 2874) ? 1 : -1];

#define GFX_RS_SK_B_LM_HEIGHT_FOG_FACTORED_SOURCE \
	"DWORD\tBcop;\n" \
	"DWORD\tBaop;\n" \
	"DWORD\tLMcop;\n" \
	"DWORD\tLMaop;\n" \
	"DWORD\tTFcop;\n" \
	"DWORD\tTFaop;\n" \
	"DWORD\talphatest;\n" \
	"DWORD\tbLighting;\n" \
	"DWORD\tbFog;\n" \
	"DWORD\tDiffuseSource;\n" \
	"MATRIX\tfogmatrix;\n" \
	"Texture\tHFog;\n" \
	"Texture\tBase;\n" \
	"Texture\tLM;\n" \
	"Technique ffp_hfog\n" \
	"{\n" \
	"Pass p0\n" \
	"{\n" \
	"AlphaTestEnable\t\t\t= < alphatest >;\n" \
	"Lighting\t\t\t\t\t= < bLighting >;\n" \
	"FogEnable\t\t\t\t\t= < bFog >;\n" \
	"DiffuseMaterialSource\t\t= < DiffuseSource >;\n" \
	"AmbientMaterialSource\t\t= < DiffuseSource >;\n" \
	"Ambient\t\t\t\t\t= 0x00;\n" \
	"ColorOp[ 0 ]\t\t\t\t= < TFcop >;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < TFaop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"Texture[ 1 ]\t\t\t\t= < LM >;\n" \
	"ColorOp[ 1 ]\t\t\t\t= < LMcop >;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= < LMaop >;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
	"TexCoordIndex[ 2 ]\t\t\t= 0;\n" \
	"Texture[ 2 ]\t\t\t\t= < Base >;\n" \
	"ColorOp[ 2 ]\t\t\t\t= < Bcop >;\n" \
	"ColorArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 2 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 2 ]\t\t\t\t= Current;\n" \
	"Texture[ 3 ]\t\t\t\t= < HFog >;\n" \
	"TextureTransform[ 3 ]\t\t= < fogmatrix >;\n" \
	"TexCoordIndex[ 3 ]\t\t\t= CameraSpacePosition | 3;\n" \
	"TextureTransformFlags[ 3 ]\t= Count2;\n" \
	"AddressU[ 3 ]\t\t\t\t= Clamp;\n" \
	"AddressV[ 3 ]\t\t\t\t= Clamp;\n" \
	"Mipfilter[ 3 ]\t\t\t\t= None;\n" \
	"ColorOp[ 3 ]\t\t\t\t= BlendTextureAlpha;\n" \
	"ColorArg1[ 3 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 3 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 3 ]\t\t\t\t= SelectArg2;\n" \
	"AlphaArg1[ 3 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 3 ]\t\t\t\t= Current;\n" \
	"ColorOp[ 4 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 4 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"}\n" \
	"Technique ffp_hfog_2pass\n" \
	"{\n" \
	"Pass p0\n" \
	"{\n" \
	"AlphaTestEnable\t\t\t= < alphatest >;\n" \
	"Lighting\t\t\t\t\t= < bLighting >;\n" \
	"FogEnable\t\t\t\t\t= < bFog >;\n" \
	"DiffuseMaterialSource\t\t= < DiffuseSource >;\n" \
	"AmbientMaterialSource\t\t= < DiffuseSource >;\n" \
	"Ambient\t\t\t\t\t= 0x00;\n" \
	"ColorOp[ 0 ]\t\t\t\t= < TFcop >;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < TFaop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"Texture[ 1 ]\t\t\t\t= < LM >;\n" \
	"ColorOp[ 1 ]\t\t\t\t= < LMcop >;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= < LMaop >;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
	"TexCoordIndex[ 2 ]\t\t\t= 0;\n" \
	"Texture[ 2 ]\t\t\t\t= < Base >;\n" \
	"ColorOp[ 2 ]\t\t\t\t= < Bcop >;\n" \
	"ColorArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 2 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 2 ]\t\t\t\t= Current;\n" \
	"ColorOp[ 3 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 3 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"Pass p1\n" \
	"{\n" \
	"AlphaBlendEnable\t\t\t= TRUE;\n" \
	"SrcBlend\t\t\t\t\t= SrcAlpha;\n" \
	"DestBlend\t\t\t\t\t= InvSrcAlpha;\n" \
	"BlendOp\t\t\t\t\t= Add;\n" \
	"ColorOp[ 0 ]\t\t\t\t= < TFcop >;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < TFaop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"Texture[ 1 ]\t\t\t\t= < Base >;\n" \
	"TexCoordIndex[ 1 ]\t\t\t= 0;\n" \
	"ColorOp[ 1 ]\t\t\t\t= SelectArg2;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Diffuse;\n" \
	"Texture[ 2 ]\t\t\t\t= < HFog >;\n" \
	"TextureTransform[ 2 ]\t\t= < fogmatrix >;\n" \
	"TexCoordIndex[ 2 ]\t\t\t= CameraSpacePosition | 1;\n" \
	"TextureTransformFlags[ 2 ]\t= Count2;\n" \
	"AddressU[ 2 ]\t\t\t\t= Clamp;\n" \
	"AddressV[ 2 ]\t\t\t\t= Clamp;\n" \
	"Mipfilter[ 2 ]\t\t\t\t= None;\n" \
	"ColorOp[ 2 ]\t\t\t\t= SelectArg1;\n" \
	"ColorArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 2 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= Modulate;\n" \
	"AlphaArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 2 ]\t\t\t\t= Current;\n" \
	"ColorOp[ 3 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 3 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"}\n" \
	"Technique ffp_hfog_3pass\n" \
	"{\n" \
	"Pass p0\n" \
	"{\n" \
	"AlphaTestEnable\t\t\t= < alphatest >;\n" \
	"Lighting\t\t\t\t\t= < bLighting >;\n" \
	"FogEnable\t\t\t\t\t= < bFog >;\n" \
	"DiffuseMaterialSource\t\t= < DiffuseSource >;\n" \
	"AmbientMaterialSource\t\t= < DiffuseSource >;\n" \
	"Ambient\t\t\t\t\t= 0x00;\n" \
	"ColorOp[ 0 ]\t\t\t\t= < TFcop >;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < TFaop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"Texture[ 1 ]\t\t\t\t= < LM >;\n" \
	"ColorOp[ 1 ]\t\t\t\t= < LMcop >;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= < LMaop >;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
	"ColorOp[ 3 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 3 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"Pass p1\n" \
	"{\n" \
	"AlphaBlendEnable\t\t\t= TRUE;\n" \
	"SrcBlend\t\t\t\t\t= DestColor;\n" \
	"DestBlend\t\t\t\t\t= SrcColor;\n" \
	"BlendOp\t\t\t\t\t= Add;\n" \
	"ColorOp[ 0 ]\t\t\t\t= < TFcop >;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < TFaop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"Texture[ 1 ]\t\t\t\t= < Base >;\n" \
	"TexCoordIndex[ 1 ]\t\t\t= 0;\n" \
	"ColorOp[ 1 ]\t\t\t\t= SelectArg1;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
	"ColorOp[ 2 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"Pass p2\n" \
	"{\n" \
	"AlphaBlendEnable\t\t\t= TRUE;\n" \
	"SrcBlend\t\t\t\t\t= SrcAlpha;\n" \
	"DestBlend\t\t\t\t\t= InvSrcAlpha;\n" \
	"BlendOp\t\t\t\t\t= Add;\n" \
	"Texture[ 0 ]\t\t\t\t= < Base >;\n" \
	"ColorOp[ 0 ]\t\t\t\t= SelectArg2;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"Texture[ 1 ]\t\t\t\t= < HFog >;\n" \
	"TextureTransform[ 1 ]\t\t= < fogmatrix >;\n" \
	"TexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\n" \
	"TextureTransformFlags[ 1 ]\t= Count2;\n" \
	"AddressU[ 1 ]\t\t\t\t= Clamp;\n" \
	"AddressV[ 1 ]\t\t\t\t= Clamp;\n" \
	"Mipfilter[ 1 ]\t\t\t\t= None;\n" \
	"ColorOp[ 1 ]\t\t\t\t= SelectArg1;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= Modulate;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
	"ColorOp[ 2 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"}\n"
	typedef char factoredSourceLength[(sizeof(GFX_RS_SK_B_LM_HEIGHT_FOG_FACTORED_SOURCE) - 1 == 5377) ? 1 : -1];


}

Gfx_RS_SK_B_LM_HeightFog::Gfx_RS_SK_B_LM_HeightFog(DWORD color, DWORD alpha,
	DWORD lightmapColor, DWORD lightmapAlpha, DWORD test)
{
	colorOperation = color;
	alphaOperation = alpha;
	lightmapColorOperation = lightmapColor;
	effect = NULL;
	lightmapAlphaOperation = lightmapAlpha;
	alphaTest = test;
}

Gfx_RS_SK_B_LM_HeightFog::~Gfx_RS_SK_B_LM_HeightFog() { Release(); }

HRESULT Gfx_RS_SK_B_LM_HeightFog::Initialize()
{
	HRESULT result = D3DERR_INVALIDCALL;
	++g_RSSKBaseLightmapHeightFogState.users;
	ID3DXBuffer* errors;
	if (!g_RSSKBaseLightmapHeightFogState.normal)
	{
		result = D3DXCreateEffect(g_D3D.device, GFX_RS_SK_B_LM_HEIGHT_FOG_NORMAL_SOURCE, sizeof(GFX_RS_SK_B_LM_HEIGHT_FOG_NORMAL_SOURCE) - 1,
			&g_RSSKBaseLightmapHeightFogState.normal, &errors);
		if (SUCCEEDED(result))
		{
			g_RSSKBaseLightmapHeightFogState.normal->GetDesc(&g_RSSKBaseLightmapHeightFogState.description);
			g_RSSKBaseLightmapHeightFogState.normal->SetTechnique(0);
			D3DXMATRIX identity = g_D3D.identity;
			g_RSSKBaseLightmapHeightFogState.normal->SetDword("alphatest", 0);
			g_RSSKBaseLightmapHeightFogState.normal->SetDword("bLighting", 0);
			g_RSSKBaseLightmapHeightFogState.normal->SetDword("bFog", 0);
			g_RSSKBaseLightmapHeightFogState.normal->SetDword("Bcop", colorOperation);
			g_RSSKBaseLightmapHeightFogState.normal->SetDword("Baop", alphaOperation);
			g_RSSKBaseLightmapHeightFogState.normal->SetDword("LMcop", lightmapColorOperation);
			g_RSSKBaseLightmapHeightFogState.normal->SetDword("LMaop", lightmapAlphaOperation);
			g_RSSKBaseLightmapHeightFogState.normal->SetDword("TFcop", 2);
			g_RSSKBaseLightmapHeightFogState.normal->SetDword("TFaop", 2);
			g_RSSKBaseLightmapHeightFogState.normal->SetDword("DiffuseSource", 1);
			g_RSSKBaseLightmapHeightFogState.normal->SetMatrix("fogmatrix", &identity);
			g_RSSKBaseLightmapHeightFogState.normal->SetTexture("HFog", NULL);
			g_RSSKBaseLightmapHeightFogState.normal->SetTexture("Base", NULL);
			g_RSSKBaseLightmapHeightFogState.normal->SetTexture("LM", NULL);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSSKBaseLightmapHeightFogState.normal->FindNextValidTechnique(0, &selected);
			if (SUCCEEDED(result))
			{
				g_RSSKBaseLightmapHeightFogState.technique = selected.Index;
				result = g_RSSKBaseLightmapHeightFogState.normal->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h') g_RSSKBaseLightmapHeightFogState.usesShader = 1;
				g_RSSKBaseLightmapHeightFogState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_SK_B_LM_HeightFog::Create - Failed to find valid technique (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_SK_B_LM_HeightFog - Failed to compile\n");
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors)
		{
			errors->Release();
			errors = NULL;
		}
	}
	if (!g_RSSKBaseLightmapHeightFogState.factored)
	{
		result = D3DXCreateEffect(g_D3D.device, GFX_RS_SK_B_LM_HEIGHT_FOG_FACTORED_SOURCE, sizeof(GFX_RS_SK_B_LM_HEIGHT_FOG_FACTORED_SOURCE) - 1,
			&g_RSSKBaseLightmapHeightFogState.factored, &errors);
		if (SUCCEEDED(result))
		{
			g_RSSKBaseLightmapHeightFogState.factored->GetDesc(&g_RSSKBaseLightmapHeightFogState.description);
			g_RSSKBaseLightmapHeightFogState.factored->SetTechnique(0);
			D3DXMATRIX identity = g_D3D.identity;
			g_RSSKBaseLightmapHeightFogState.factored->SetDword("alphatest", 0);
			g_RSSKBaseLightmapHeightFogState.factored->SetDword("bLighting", 0);
			g_RSSKBaseLightmapHeightFogState.factored->SetDword("bFog", 0);
			g_RSSKBaseLightmapHeightFogState.factored->SetDword("Bcop", colorOperation);
			g_RSSKBaseLightmapHeightFogState.factored->SetDword("Baop", alphaOperation);
			g_RSSKBaseLightmapHeightFogState.factored->SetDword("LMcop", lightmapColorOperation);
			g_RSSKBaseLightmapHeightFogState.factored->SetDword("LMaop", lightmapAlphaOperation);
			g_RSSKBaseLightmapHeightFogState.factored->SetDword("TFcop", 2);
			g_RSSKBaseLightmapHeightFogState.factored->SetDword("TFaop", 2);
			g_RSSKBaseLightmapHeightFogState.factored->SetDword("DiffuseSource", 1);
			g_RSSKBaseLightmapHeightFogState.factored->SetMatrix("fogmatrix", &identity);
			g_RSSKBaseLightmapHeightFogState.factored->SetTexture("HFog", NULL);
			g_RSSKBaseLightmapHeightFogState.factored->SetTexture("Base", NULL);
			g_RSSKBaseLightmapHeightFogState.factored->SetTexture("LM", NULL);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSSKBaseLightmapHeightFogState.factored->FindNextValidTechnique(0, &selected);
			if (SUCCEEDED(result))
			{
				g_RSSKBaseLightmapHeightFogState.technique = selected.Index;
				result = g_RSSKBaseLightmapHeightFogState.factored->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h') g_RSSKBaseLightmapHeightFogState.usesShader = 1;
				if (g_RSSKBaseLightmapHeightFogState.passes < selected.Passes) g_RSSKBaseLightmapHeightFogState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_SK_B_LM_HeightFog::Create - Failed to find valid technique (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_SK_B_LM_HeightFog - Failed to compile\n");
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors)
		{
			errors->Release();
		}
	}
	return result;
}

void Gfx_RS_SK_B_LM_HeightFog::Release()
{
	Gfx_RS_SK_B_LM_HeightFogState& state = g_RSSKBaseLightmapHeightFogState;
	if (state.users)
	{
		--state.users;
		if (!state.users)
		{
			if (state.normal) { state.normal->Release(); state.normal = NULL; }
			if (state.factored) { state.factored->Release(); state.factored = NULL; }
		}
	}
}

HRESULT Gfx_RS_SK_B_LM_HeightFog::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSSKBaseLightmapHeightFogState.normal) result = g_RSSKBaseLightmapHeightFogState.normal->OnLostDevice();
	if (g_RSSKBaseLightmapHeightFogState.factored) result = g_RSSKBaseLightmapHeightFogState.factored->OnLostDevice();
	return result;
}
HRESULT Gfx_RS_SK_B_LM_HeightFog::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSSKBaseLightmapHeightFogState.normal) result = g_RSSKBaseLightmapHeightFogState.normal->OnResetDevice();
	if (g_RSSKBaseLightmapHeightFogState.factored) result = g_RSSKBaseLightmapHeightFogState.factored->OnResetDevice();
	return result;
}
DWORD Gfx_RS_SK_B_LM_HeightFog::TechniqueCount() { return g_RSSKBaseLightmapHeightFogState.description.Techniques; }
BYTE Gfx_RS_SK_B_LM_HeightFog::UsesVertexShader() { return g_RSSKBaseLightmapHeightFogState.usesShader; }
DWORD Gfx_RS_SK_B_LM_HeightFog::PassCount() { return g_RSSKBaseLightmapHeightFogState.passes; }
DWORD Gfx_RS_SK_B_LM_HeightFog::SelectTechnique(DWORD index)
{
	Gfx_RS_SK_B_LM_HeightFogState& state = g_RSSKBaseLightmapHeightFogState;
	if (index < state.description.Techniques && index != reinterpret_cast<DWORD>(state.technique))
	{
		state.techniqueChanged = 1;
		state.technique = reinterpret_cast<LPCSTR>(index);
	}
	return index;
}
DWORD Gfx_RS_SK_B_LM_HeightFog::NextTechnique()
{
	DWORD next = reinterpret_cast<DWORD>(g_RSSKBaseLightmapHeightFogState.technique) + 1;
	if (next >= g_RSSKBaseLightmapHeightFogState.description.Techniques) next = 0;
	return SelectTechnique(next);
}
HRESULT Gfx_RS_SK_B_LM_HeightFog::Begin(UINT* passes) { return effect->Begin(passes, 0); }
HRESULT Gfx_RS_SK_B_LM_HeightFog::End() { return effect->End(); }
HRESULT Gfx_RS_SK_B_LM_HeightFog::Pass(UINT pass)
{
	g_RSSKBaseLightmapHeightFogState.currentPass = pass;
	return effect->Pass(pass);
}

HRESULT Gfx_RS_SK_B_LM_HeightFog::SetMaterial(Gfx_Material* material, DWORD fogBits)
{
	BYTE useRGB = 0, useAlpha = 0;
	DWORD color;
	material->GetColor(useRGB, useAlpha, color);
	effect = useRGB || useAlpha ? g_RSSKBaseLightmapHeightFogState.factored : g_RSSKBaseLightmapHeightFogState.normal;
	if (!effect)
	{
		JE::LogDebugString("Gfx_RS_SK_B_LM_HeightFog::SetMaterial() failed - not created yet.");
		return D3DERR_INVALIDCALL;
	}
	Gfx_HeightFog* fog = reinterpret_cast<Gfx_HeightFog*>(fogBits);
	if (!fog)
	{
		JE::LogDebugString("Gfx_RS_SK_B_LM_HeightFog: No HeightFog object specified!");
		return D3DERR_INVALIDCALL;
	}
	DWORD handle = material->textureHandles[1];
	if (handle)
	{
		Gfx_Texture* texture = g_TextureManager->Lookup(handle);
		effect->SetTexture("LM", texture && texture->frameCount > 0 ? texture->frames[0].texture : NULL);
		if (!texture)
		{
			const std::string name = Gfx_GetTextureDiagnosticName(texture); //Original NULL-object copy4AB1C0 remains invalid.
			JE::LogDebugString("Gfx_RS_SK_B_LM_HeightFog: Cannot find Lightmap %s", name.c_str());
		}
	}
	else effect->SetTexture("LM", NULL);
	handle = material->textureHandles[0];
	if (handle)
	{
		Gfx_Texture* texture = g_TextureManager->Lookup(handle);
		effect->SetTexture("Base", texture && texture->frameCount > 0 ? texture->frames[0].texture : NULL);
		if (!texture)
		{
			const std::string name = Gfx_GetTextureDiagnosticName(texture);
			JE::LogDebugString("Gfx_RS_SK_B_LM_HeightFog: Cannot find Base Texture %s", name.c_str());
		}
	}
	else
	{
		effect->SetTexture("Base", NULL);
		JE::LogDebugString("Gfx_RS_SK_B_LM_HeightFog: NULL Base Texture handle\n");
	}
	if (fog->value44)
	{
		Gfx_Texture* texture = g_TextureManager->Lookup(fog->value44);
		effect->SetTexture("HFog", texture && texture->frameCount > 0 ? texture->frames[0].texture : NULL);
		if (!texture) JE::LogDebugString("Gfx_RS_SK_B_LM_HeightFog: Height Fog map not found!");
	}
	else
	{
		effect->SetTexture("HFog", NULL);
		JE::LogDebugString("Gfx_RS_SK_B_LM_HeightFog: Height Fog map not created!");
	}
	effect->SetDword("alphatest", alphaTest);
	effect->SetDword("bLighting", material->flagBC ? g_GfxSystem->Lighting() != 0 : 0);
	effect->SetDword("bFog", material->flagBD ? g_GfxSystem->CurrentViewport()->fogEnabled != 0 : 0);
	effect->SetDword("DiffuseSource", material->flagB8 != 0);
	effect->SetDword("Bcop", colorOperation);
	effect->SetDword("Baop", alphaOperation);
	effect->SetDword("LMcop", lightmapColorOperation);
	effect->SetDword("LMaop", lightmapAlphaOperation);
	D3DMATERIAL8 values;
	std::memcpy(&values, &material->values, sizeof(values));
	g_D3D.device->SetMaterial(&values);
	if (useRGB || useAlpha)
	{
		effect->SetDword("TFcop", useRGB ? 5 : 2);
		effect->SetDword("TFaop", useAlpha ? 4 : 2);
		if (!useRGB) color = (color & 0xFF808080) | 0x00808080;
		g_D3D.SetRenderState(D3DRS_TEXTUREFACTOR, color);
	}
	else
	{
		effect->SetDword("TFcop", 2);
		effect->SetDword("TFaop", 2);
	}
	HRESULT result = g_D3D.SetRenderState(D3DRS_CULLMODE, material->flagB9 ? 3 : 1);
	const float seconds = g_GfxSystem->TimeSeconds();
	float origin, range;
	fog->Evaluate(seconds, &origin, &range);
	const D3DXVECTOR3 position(fog->transform._41, fog->transform._42, fog->transform._43);
	if (g_RSSKBaseLightmapHeightFogState.usesShader)
	{
		const D3DXVECTOR4 constants(0.0f, 0.5f, 1.0f, 2.0f);
		g_D3D.device->SetVertexShaderConstant(0, &constants, 1);
		BYTE red, green, blue;
		g_GfxSystem->GetAmbientRGB(&red, &green, &blue);
		const D3DXVECTOR4 ambient(red * 0.0039215689f, green * 0.0039215689f, blue * 0.0039215689f, 1.0f);
		g_D3D.device->SetVertexShaderConstant(1, &ambient, 1);
		g_GfxSystem->CurrentViewport(); //Actual first discarded getter.
		Gfx_Viewport* viewport = g_GfxSystem->CurrentViewport();
		const D3DXVECTOR4 cameraFog(viewport->fogEnd, float(1.0 / (double(viewport->fogEnd) - viewport->fogStart)), 0.0f, 0.0f);
		g_D3D.device->SetVertexShaderConstant(2, &cameraFog, 1);
		g_D3D.device->SetVertexShaderConstant(3, &values.Diffuse, 1);
		g_D3D.device->SetVertexShaderConstant(4, &values.Emissive, 1);
		const D3DXVECTOR4 location(position.x, position.y, position.z, 0.0f);
		g_D3D.device->SetVertexShaderConstant(40, &location, 1);
		const D3DXVECTOR4 dimensions(1.0f, float(1.0 / double(range)), 0.0066666668f, 0.0034482758f);
		result = g_D3D.device->SetVertexShaderConstant(41, &dimensions, 1);
	}
	else
	{
		D3DXMATRIX map = g_D3D.identity;
		D3DXMATRIX inverseView;
		g_GfxSystem->Camera()->GetInverseViewMatrix(inverseView);
		const double inverseRange = 1.0 / double(range);
		map._11 = 0.0f;
		map._33 = 0.0f;
		map._22 = float(inverseRange);
		map._42 = float(-((double(position.y) + origin) * inverseRange));
		D3DXMATRIX product;
		D3DXMatrixMultiply(&product, &inverseView, &map);
		D3DXMATRIX matrix = product;
		effect->SetMatrix("fogmatrix", &matrix);
	}
	DWORD flags = material->values140[0];
	if (flags) Gfx_SetTextureTransform(1, static_cast<BYTE>(flags), &material->textureParameters[0], seconds);
	else g_D3D.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	if (g_RSSKBaseLightmapHeightFogState.techniqueChanged)
	{
		D3DXTECHNIQUE_DESC selected;
		result = effect->FindNextValidTechnique(0, &selected);
		if (FAILED(result))
			JE::FatalError("Gfx_RS_SK_B_LM_HeightFog::SetMaterial - Failed to find valid technique (%x)\n", result);
		g_RSSKBaseLightmapHeightFogState.techniqueChanged = 0;
		g_RSSKBaseLightmapHeightFogState.technique = selected.Index;
		g_RSSKBaseLightmapHeightFogState.passes = selected.Passes;
		result = effect->SetTechnique(selected.Index);
	}
	return result;
}
