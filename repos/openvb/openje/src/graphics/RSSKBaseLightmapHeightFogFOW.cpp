#include "RSSKBaseLightmapHeightFogFOW.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "Material.hpp"
#include "HeightFog.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"
#include "Camera.hpp"
#include "TextureTransform.hpp"
#include "FogOfWarTextureMatrix.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

Gfx_RS_SK_B_LM_HFog_FOWState g_RSSKBaseLightmapHeightFogFOWState = {0};

namespace
{
	// Exact original FX byte strings68D030/68E7C8 are embedded below.
#define GFX_RS_SK_B_LM_HFOG_FOW_NORMAL_SOURCE \
	"DWORD\t\tBcop;\n" \
	"DWORD\t\tBaop;\n" \
	"DWORD\t\tLMcop;\n" \
	"DWORD\t\tLMaop;\n" \
	"DWORD\t\tTFcop;\n" \
	"DWORD\t\tTFaop;\n" \
	"DWORD\t\tFOWcop;\n" \
	"DWORD\t\tFOWaop;\n" \
	"DWORD\t\talphatest;\n" \
	"DWORD\t\tbLighting;\n" \
	"DWORD\t\tbFog;\n" \
	"DWORD\t\tDiffuseSource;\n" \
	"MATRIX\t\tfogmatrix;\n" \
	"Texture\tHFog;\n" \
	"MATRIX\t\tFOWmatrix;\n" \
	"Texture\tFOW;\n" \
	"Texture\tBase;\n" \
	"Texture\tLM;\n" \
	"Technique ffp\n" \
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
	"Texture[ 3 ]\t\t\t\t= < FOW >;\n" \
	"TextureTransform[ 3 ]\t\t= < FOWmatrix >;\n" \
	"TexCoordIndex[ 3 ]\t\t\t= CameraSpacePosition | 3;\n" \
	"TextureTransformFlags[ 3 ]\t= Count2;\n" \
	"AddressU[ 3 ]\t\t\t\t= Clamp;\n" \
	"AddressV[ 3 ]\t\t\t\t= Clamp;\n" \
	"Mipfilter[ 3 ]\t\t\t\t= None;\n" \
	"ColorOp[ 3 ]\t\t\t\t= < FOWcop >;\n" \
	"ColorArg1[ 3 ]\t\t\t\t= Current;\n" \
	"ColorArg2[ 3 ]\t\t\t\t= Texture;\n" \
	"AlphaOp[ 3 ]\t\t\t\t= < FOWaop >;\n" \
	"AlphaArg1[ 3 ]\t\t\t\t= Current;\n" \
	"AlphaArg2[ 3 ]\t\t\t\t= Texture;\n" \
	"ColorOp[ 4 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 4 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"}\n" \
	"Technique ffp_2pass\n" \
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
	"Pass p1\n" \
	"{\n" \
	"AlphaBlendEnable\t\t\t= TRUE;\n" \
	"SrcBlend\t\t\t\t\t= DestColor;\n" \
	"DestBlend\t\t\t\t\t= Zero;\n" \
	"BlendOp\t\t\t\t\t= Add;\n" \
	"Texture[ 0 ]\t\t\t\t= < Base >;\n" \
	"ColorOp[ 0 ]\t\t\t\t= SelectArg2;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"Texture[ 1 ]\t\t\t\t= < FOW >;\n" \
	"TextureTransform[ 1 ]\t\t= < FOWmatrix >;\n" \
	"TexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\n" \
	"TextureTransformFlags[ 1 ]\t= Count2;\n" \
	"AddressU[ 1 ]\t\t\t\t= Clamp;\n" \
	"AddressV[ 1 ]\t\t\t\t= Clamp;\n" \
	"Mipfilter[ 1 ]\t\t\t\t= None;\n" \
	"ColorOp[ 1 ]\t\t\t\t= SelectArg2;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Current;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= SelectArg1;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorOp[ 2 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"}\n" \
	"Technique ffp_3pass\n" \
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
	"Pass p2\n" \
	"{\n" \
	"AlphaBlendEnable\t\t\t= TRUE;\n" \
	"SrcBlend\t\t\t\t\t= DestColor;\n" \
	"DestBlend\t\t\t\t\t= Zero;\n" \
	"BlendOp\t\t\t\t\t= Add;\n" \
	"Texture[ 0 ]\t\t\t\t= < Base >;\n" \
	"ColorOp[ 0 ]\t\t\t\t= SelectArg2;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"Texture[ 1 ]\t\t\t\t= < FOW >;\n" \
	"TextureTransform[ 1 ]\t\t= < FOWmatrix >;\n" \
	"TexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\n" \
	"TextureTransformFlags[ 1 ]\t= Count2;\n" \
	"AddressU[ 1 ]\t\t\t\t= Clamp;\n" \
	"AddressV[ 1 ]\t\t\t\t= Clamp;\n" \
	"Mipfilter[ 1 ]\t\t\t\t= None;\n" \
	"ColorOp[ 1 ]\t\t\t\t= SelectArg2;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Current;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= Modulate;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorOp[ 2 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"}\n"
	typedef char normalSourceLength[(sizeof(GFX_RS_SK_B_LM_HFOG_FOW_NORMAL_SOURCE) - 1 == 6032) ? 1 : -1];

#define GFX_RS_SK_B_LM_HFOG_FOW_FACTORED_SOURCE \
	"DWORD\t\tBcop;\n" \
	"DWORD\t\tBaop;\n" \
	"DWORD\t\tLMcop;\n" \
	"DWORD\t\tLMaop;\n" \
	"DWORD\t\tTFcop;\n" \
	"DWORD\t\tTFaop;\n" \
	"DWORD\t\tFOWcop;\n" \
	"DWORD\t\tFOWaop;\n" \
	"DWORD\t\talphatest;\n" \
	"DWORD\t\tbLighting;\n" \
	"DWORD\t\tbFog;\n" \
	"DWORD\t\tDiffuseSource;\n" \
	"MATRIX\t\tfogmatrix;\n" \
	"Texture\tHFog;\n" \
	"MATRIX\t\tFOWmatrix;\n" \
	"Texture\tFOW;\n" \
	"Texture\tBase;\n" \
	"Texture\tLM;\n" \
	"Technique ffp\n" \
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
	"ColorArg1[ 0 ]\t\t\t\t= Current;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < TFaop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Current;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"Texture[ 1 ]\t\t\t\t= < LM >;\n" \
	"ColorOp[ 1 ]\t\t\t\t= < LMcop >;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= < LMaop >;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
	"Texture[ 2 ]\t\t\t\t= < Base >;\n" \
	"TexCoordIndex[ 2 ]\t\t\t= 0;\n" \
	"ColorOp[ 2 ]\t\t\t\t= < Bcop >;\n" \
	"ColorArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 2 ]\t\t\t\t= Diffuse;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 2 ]\t\t\t\t= Diffuse;\n" \
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
	"Texture[ 4 ]\t\t\t\t= < FOW >;\n" \
	"TextureTransform[ 4 ]\t\t= < FOWmatrix >;\n" \
	"TexCoordIndex[ 4 ]\t\t\t= CameraSpacePosition | 4;\n" \
	"TextureTransformFlags[ 4 ]\t= Count2;\n" \
	"AddressU[ 4 ]\t\t\t\t= Clamp;\n" \
	"AddressV[ 4 ]\t\t\t\t= Clamp;\n" \
	"Mipfilter[ 4 ]\t\t\t\t= None;\n" \
	"ColorOp[ 4 ]\t\t\t\t= < FOWcop >;\n" \
	"ColorArg1[ 4 ]\t\t\t\t= Current;\n" \
	"ColorArg2[ 4 ]\t\t\t\t= Texture;\n" \
	"AlphaOp[ 4 ]\t\t\t\t= < FOWaop >;\n" \
	"AlphaArg1[ 4 ]\t\t\t\t= Current;\n" \
	"AlphaArg2[ 4 ]\t\t\t\t= Texture;\n" \
	"ColorOp[ 5 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 5 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"}\n" \
	"Technique ffp_2pass\n" \
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
	"ColorArg1[ 0 ]\t\t\t\t= Current;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < TFaop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Current;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"Texture[ 1 ]\t\t\t\t= < LM >;\n" \
	"ColorOp[ 1 ]\t\t\t\t= < LMcop >;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= < LMaop >;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
	"Texture[ 2 ]\t\t\t\t= < Base >;\n" \
	"TexCoordIndex[ 2 ]\t\t\t= 0;\n" \
	"ColorOp[ 2 ]\t\t\t\t= < Bcop >;\n" \
	"ColorArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 2 ]\t\t\t\t= Diffuse;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 2 ]\t\t\t\t= Diffuse;\n" \
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
	"Pass\tp1\n" \
	"{\n" \
	"AlphaBlendEnable\t\t\t= TRUE;\n" \
	"SrcBlend\t\t\t\t\t= DestColor;\n" \
	"DestBlend\t\t\t\t\t= Zero;\n" \
	"BlendOp\t\t\t\t\t= Add;\n" \
	"Texture[ 0 ]\t\t\t\t= < Base >;\n" \
	"ColorOp[ 0 ]\t\t\t\t= SelectArg2;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"Texture[ 1 ]\t\t\t\t= < FOW >;\n" \
	"TextureTransform[ 1 ]\t\t= < FOWmatrix >;\n" \
	"TexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\n" \
	"TextureTransformFlags[ 1 ]\t= Count2;\n" \
	"AddressU[ 1 ]\t\t\t\t= Clamp;\n" \
	"AddressV[ 1 ]\t\t\t\t= Clamp;\n" \
	"Mipfilter[ 1 ]\t\t\t\t= None;\n" \
	"ColorOp[ 1 ]\t\t\t\t= SelectArg2;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Current;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= Modulate;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorOp[ 2 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"}\n" \
	"Technique ffp_3pass\n" \
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
	"ColorArg1[ 0 ]\t\t\t\t= Current;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < TFaop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Current;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"Texture[ 1 ]\t\t\t\t= < LM >;\n" \
	"ColorOp[ 1 ]\t\t\t\t= < LMcop >;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= < LMaop >;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
	"Texture[ 2 ]\t\t\t\t= < Base >;\n" \
	"TexCoordIndex[ 2 ]\t\t\t= 0;\n" \
	"ColorOp[ 2 ]\t\t\t\t= < Bcop >;\n" \
	"ColorArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 2 ]\t\t\t\t= Diffuse;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 2 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 2 ]\t\t\t\t= Diffuse;\n" \
	"ColorOp[ 3 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 3 ]\t\t\t\t= Disable;\n" \
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
	"Pass p2\n" \
	"{\n" \
	"AlphaBlendEnable\t\t\t= TRUE;\n" \
	"SrcBlend\t\t\t\t\t= DestColor;\n" \
	"DestBlend\t\t\t\t\t= Zero;\n" \
	"BlendOp\t\t\t\t\t= Add;\n" \
	"Texture[ 0 ]\t\t\t\t= < FOW >;\n" \
	"Texture[ 0 ]\t\t\t\t= < Base >;\n" \
	"ColorOp[ 0 ]\t\t\t\t= SelectArg2;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"TextureTransform[ 1 ]\t\t= < FOWmatrix >;\n" \
	"TexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\n" \
	"TextureTransformFlags[ 1 ]\t= Count2;\n" \
	"AddressU[ 1 ]\t\t\t\t= Clamp;\n" \
	"AddressV[ 1 ]\t\t\t\t= Clamp;\n" \
	"Mipfilter[ 1 ]\t\t\t\t= None;\n" \
	"ColorOp[ 1 ]\t\t\t\t= SelectArg2;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Current;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= SelectArg1;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorOp[ 2 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"}\n" \
	"Technique ffp_4pass\n" \
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
	"ColorArg1[ 0 ]\t\t\t\t= Current;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < TFaop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Current;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Tfactor;\n" \
	"Texture[ 1 ]\t\t\t\t= < LM >;\n" \
	"ColorOp[ 1 ]\t\t\t\t= < LMcop >;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= < LMaop >;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Current;\n" \
	"ColorOp[ 2 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"Pass p1\n" \
	"{\n" \
	"AlphaBlendEnable\t\t\t= TRUE;\n" \
	"SrcBlend\t\t\t\t\t= DestColor;\n" \
	"DestBlend\t\t\t\t\t= SrcColor;\n" \
	"BlendOp\t\t\t\t\t= Add;\n" \
	"Texture[ 0 ]\t\t\t\t= < Base >;\n" \
	"ColorOp[ 0 ]\t\t\t\t= < Bcop >;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"ColorOp[ 1 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= Disable;\n" \
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
	"Pass p3\n" \
	"{\n" \
	"AlphaBlendEnable\t\t\t= TRUE;\n" \
	"SrcBlend\t\t\t\t\t= DestColor;\n" \
	"DestBlend\t\t\t\t\t= Zero;\n" \
	"BlendOp\t\t\t\t\t= Add;\n" \
	"Texture[ 0 ]\t\t\t\t= < Base >;\n" \
	"ColorOp[ 0 ]\t\t\t\t= SelectArg2;\n" \
	"ColorArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"ColorArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"AlphaOp[ 0 ]\t\t\t\t= < Baop >;\n" \
	"AlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
	"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\n" \
	"Texture[ 1 ]\t\t\t\t= < FOW >;\n" \
	"TextureTransform[ 1 ]\t\t= < FOWmatrix >;\n" \
	"TexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\n" \
	"TextureTransformFlags[ 1 ]\t= Count2;\n" \
	"AddressU[ 1 ]\t\t\t\t= Clamp;\n" \
	"AddressV[ 1 ]\t\t\t\t= Clamp;\n" \
	"Mipfilter[ 1 ]\t\t\t\t= None;\n" \
	"ColorOp[ 1 ]\t\t\t\t= SelectArg2;\n" \
	"ColorArg1[ 1 ]\t\t\t\t= Current;\n" \
	"ColorArg2[ 1 ]\t\t\t\t= Texture;\n" \
	"AlphaOp[ 1 ]\t\t\t\t= Modulate;\n" \
	"AlphaArg1[ 1 ]\t\t\t\t= Current;\n" \
	"AlphaArg2[ 1 ]\t\t\t\t= Texture;\n" \
	"ColorOp[ 2 ]\t\t\t\t= Disable;\n" \
	"AlphaOp[ 2 ]\t\t\t\t= Disable;\n" \
	"}\n" \
	"}\n"
	typedef char factoredSourceLength[(sizeof(GFX_RS_SK_B_LM_HFOG_FOW_FACTORED_SOURCE) - 1 == 9042) ? 1 : -1];


}

Gfx_RS_SK_B_LM_HFog_FOW::Gfx_RS_SK_B_LM_HFog_FOW(DWORD color, DWORD alpha,
	DWORD lightmapColor, DWORD lightmapAlpha, DWORD test)
{
	colorOperation = color;
	alphaOperation = alpha;
	lightmapColorOperation = lightmapColor;
	effect = NULL;
	lightmapAlphaOperation = lightmapAlpha;
	alphaTest = test;
}

Gfx_RS_SK_B_LM_HFog_FOW::~Gfx_RS_SK_B_LM_HFog_FOW() { Release(); }

HRESULT Gfx_RS_SK_B_LM_HFog_FOW::Initialize()
{
	HRESULT result = D3DERR_INVALIDCALL;
	++g_RSSKBaseLightmapHeightFogFOWState.users;
	ID3DXBuffer* errors;
	if (!g_RSSKBaseLightmapHeightFogFOWState.normal)
	{
		result = D3DXCreateEffect(g_D3D.device, GFX_RS_SK_B_LM_HFOG_FOW_NORMAL_SOURCE, sizeof(GFX_RS_SK_B_LM_HFOG_FOW_NORMAL_SOURCE) - 1,
			&g_RSSKBaseLightmapHeightFogFOWState.normal, &errors);
		if (SUCCEEDED(result))
		{
			g_RSSKBaseLightmapHeightFogFOWState.normal->GetDesc(&g_RSSKBaseLightmapHeightFogFOWState.description);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetTechnique(0);
			D3DXMATRIX identity = g_D3D.identity;
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetDword("alphatest", 0);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetDword("bLighting", 0);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetDword("bFog", 0);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetDword("Bcop", colorOperation);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetDword("Baop", alphaOperation);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetDword("LMcop", lightmapColorOperation);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetDword("LMaop", lightmapAlphaOperation);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetDword("TFcop", 2);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetDword("TFaop", 2);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetDword("FOWcop", 2);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetDword("FOWaop", 2);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetDword("DiffuseSource", 1);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetMatrix("fogmatrix", &identity);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetTexture("HFog", NULL);
			g_RSSKBaseLightmapHeightFogFOWState.normal->SetTexture("FOW", NULL);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSSKBaseLightmapHeightFogFOWState.normal->FindNextValidTechnique(g_RSSKBaseLightmapHeightFogFOWState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSSKBaseLightmapHeightFogFOWState.technique = selected.Index;
				result = g_RSSKBaseLightmapHeightFogFOWState.normal->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h') g_RSSKBaseLightmapHeightFogFOWState.usesShader = 1;
				g_RSSKBaseLightmapHeightFogFOWState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_SK_B_LM_HFog_FOW::Create - Failed to find valid technique (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_SK_B_LM_HFog_FOW - Failed to compile\n");
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors)
		{
			errors->Release();
			errors = NULL;
		}
	}
	if (!g_RSSKBaseLightmapHeightFogFOWState.factored)
	{
		result = D3DXCreateEffect(g_D3D.device, GFX_RS_SK_B_LM_HFOG_FOW_FACTORED_SOURCE, sizeof(GFX_RS_SK_B_LM_HFOG_FOW_FACTORED_SOURCE) - 1,
			&g_RSSKBaseLightmapHeightFogFOWState.factored, &errors);
		if (SUCCEEDED(result))
		{
			g_RSSKBaseLightmapHeightFogFOWState.factored->GetDesc(&g_RSSKBaseLightmapHeightFogFOWState.description);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetTechnique(0);
			D3DXMATRIX identity = g_D3D.identity;
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetDword("alphatest", 0);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetDword("bLighting", 0);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetDword("bFog", 0);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetDword("Bcop", colorOperation);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetDword("Baop", alphaOperation);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetDword("LMcop", lightmapColorOperation);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetDword("LMaop", lightmapAlphaOperation);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetDword("TFcop", 2);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetDword("TFaop", 2);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetDword("FOWcop", 2);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetDword("FOWaop", 2);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetDword("DiffuseSource", 1);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetMatrix("fogmatrix", &identity);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetTexture("HFog", NULL);
			g_RSSKBaseLightmapHeightFogFOWState.factored->SetTexture("FOW", NULL);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSSKBaseLightmapHeightFogFOWState.factored->FindNextValidTechnique(g_RSSKBaseLightmapHeightFogFOWState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSSKBaseLightmapHeightFogFOWState.technique = selected.Index;
				result = g_RSSKBaseLightmapHeightFogFOWState.factored->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h') g_RSSKBaseLightmapHeightFogFOWState.usesShader = 1;
				if (g_RSSKBaseLightmapHeightFogFOWState.passes < selected.Passes) g_RSSKBaseLightmapHeightFogFOWState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_SK_B_LM_HFog_FOW::Create - Failed to find valid technique (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_SK_B_LM_HFog_FOW - Failed to compile\n");
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors)
		{
			errors->Release();
		}
	}
	return result;
}

void Gfx_RS_SK_B_LM_HFog_FOW::Release()
{
	Gfx_RS_SK_B_LM_HFog_FOWState& state = g_RSSKBaseLightmapHeightFogFOWState;
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

HRESULT Gfx_RS_SK_B_LM_HFog_FOW::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSSKBaseLightmapHeightFogFOWState.normal) result = g_RSSKBaseLightmapHeightFogFOWState.normal->OnLostDevice();
	if (g_RSSKBaseLightmapHeightFogFOWState.factored) result = g_RSSKBaseLightmapHeightFogFOWState.factored->OnLostDevice();
	return result;
}
HRESULT Gfx_RS_SK_B_LM_HFog_FOW::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSSKBaseLightmapHeightFogFOWState.normal) result = g_RSSKBaseLightmapHeightFogFOWState.normal->OnResetDevice();
	if (g_RSSKBaseLightmapHeightFogFOWState.factored) result = g_RSSKBaseLightmapHeightFogFOWState.factored->OnResetDevice();
	return result;
}
DWORD Gfx_RS_SK_B_LM_HFog_FOW::TechniqueCount() { return g_RSSKBaseLightmapHeightFogFOWState.description.Techniques; }
BYTE Gfx_RS_SK_B_LM_HFog_FOW::UsesVertexShader() { return g_RSSKBaseLightmapHeightFogFOWState.usesShader; }
DWORD Gfx_RS_SK_B_LM_HFog_FOW::PassCount() { return g_RSSKBaseLightmapHeightFogFOWState.passes; }
DWORD Gfx_RS_SK_B_LM_HFog_FOW::SelectTechnique(DWORD index)
{
	Gfx_RS_SK_B_LM_HFog_FOWState& state = g_RSSKBaseLightmapHeightFogFOWState;
	if (index < state.description.Techniques && index != reinterpret_cast<DWORD>(state.technique))
	{
		state.techniqueChanged = 1;
		state.technique = reinterpret_cast<LPCSTR>(index);
	}
	return index;
}
DWORD Gfx_RS_SK_B_LM_HFog_FOW::NextTechnique()
{
	DWORD next = reinterpret_cast<DWORD>(g_RSSKBaseLightmapHeightFogFOWState.technique) + 1;
	if (next >= g_RSSKBaseLightmapHeightFogFOWState.description.Techniques) next = 0;
	return SelectTechnique(next);
}
HRESULT Gfx_RS_SK_B_LM_HFog_FOW::Begin(UINT* passes) { return effect->Begin(passes, 0); }
HRESULT Gfx_RS_SK_B_LM_HFog_FOW::End() { return effect->End(); }
HRESULT Gfx_RS_SK_B_LM_HFog_FOW::Pass(UINT pass)
{
	g_RSSKBaseLightmapHeightFogFOWState.currentPass = pass;
	return effect->Pass(pass);
}

HRESULT Gfx_RS_SK_B_LM_HFog_FOW::SetMaterial(Gfx_Material* material, DWORD fogBits)
{
	BYTE useRGB = 0, useAlpha = 0;
	DWORD color;
	material->GetColor(useRGB, useAlpha, color);
	effect = useRGB || useAlpha ? g_RSSKBaseLightmapHeightFogFOWState.factored : g_RSSKBaseLightmapHeightFogFOWState.normal;
	if (!effect)
	{
		JE::LogDebugString("Gfx_RS_SK_B_LM_HFog_FOW::SetMaterial() failed - not created yet.");
		return D3DERR_INVALIDCALL;
	}
	Gfx_HeightFog* fog = reinterpret_cast<Gfx_HeightFog*>(fogBits);
	if (!fog)
	{
		JE::LogDebugString("Gfx_RS_SK_B_LM_HFog_FOW: No HeightFog object specified!");
		return D3DERR_INVALIDCALL;
	}
	DWORD handle = material->textureHandles[0];
	if (handle)
	{
		Gfx_Texture* texture = g_TextureManager->Lookup(handle);
		if (texture) texture->Bind(0, 0);
		else
		{
			g_D3D.SetTexture(0, NULL);
			const std::string name = Gfx_GetTextureDiagnosticName(texture); //Original NULL-object copy4AB1C0 remains invalid.
			JE::LogDebugString("Gfx_RS_SK_B_LM_HFog_FOW: Cannot find Base Texture %s", name.c_str());
		}
	}
	else
	{
		g_D3D.SetTexture(0, NULL);
		JE::LogDebugString("Gfx_RS_SK_B_LM_HFog_FOW: NULL Base Texture handle\n");
	}
	handle = material->textureHandles[1];
	if (handle)
	{
		Gfx_Texture* texture = g_TextureManager->Lookup(handle);
		if (texture) texture->Bind(1, 0);
		else
		{
			g_D3D.SetTexture(1, NULL);
			const std::string name = Gfx_GetTextureDiagnosticName(texture);
			JE::LogDebugString("Gfx_RS_SK_B_LM_HFog_FOW: Cannot find Lightmap %s", name.c_str());
		}
	}
	else g_D3D.SetTexture(1, NULL);
	if (fog->value44)
	{
		Gfx_Texture* texture = g_TextureManager->Lookup(fog->value44);
		effect->SetTexture("HFog", texture && texture->frameCount > 0 ? texture->frames[0].texture : NULL);
		if (!texture) JE::LogDebugString("Gfx_RS_SK_B_LM_HFog_FOW: Height Fog map not found!");
	}
	else
	{
		effect->SetTexture("HFog", NULL);
		JE::LogDebugString("Gfx_RS_SK_B_LM_HFog_FOW: Height Fog map not created!");
	}
	IDirect3DTexture8* fogOfWar = g_GfxSystem->FogOfWarRenderTexture();
	if (fogOfWar)
	{
		effect->SetTexture("FOW", fogOfWar);
		effect->SetDword("FOWcop", 4);
		effect->SetDword("FOWaop", 2);
	}
	else
	{
		effect->SetTexture("FOW", NULL);
		effect->SetDword("FOWcop", 1);
		effect->SetDword("FOWaop", 1);
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
	D3DXMATRIX fogOfWarMatrix;
	Gfx_BuildFogOfWarTextureMatrix(fogOfWarMatrix);
	effect->SetMatrix("FOWmatrix", &fogOfWarMatrix);
	const float seconds = g_GfxSystem->TimeSeconds();
	float origin, range;
	fog->Evaluate(seconds, &origin, &range);
	const D3DXVECTOR3 position(fog->transform._41, fog->transform._42, fog->transform._43);
	if (g_RSSKBaseLightmapHeightFogFOWState.usesShader)
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
	if (g_RSSKBaseLightmapHeightFogFOWState.techniqueChanged)
	{
		D3DXTECHNIQUE_DESC selected;
		result = effect->FindNextValidTechnique(g_RSSKBaseLightmapHeightFogFOWState.technique, &selected);
		if (FAILED(result))
			JE::FatalError("Gfx_RS_SK_B_LM_HFog_FOW::SetMaterial - Failed to find valid technique (%x)\n", result);
		g_RSSKBaseLightmapHeightFogFOWState.techniqueChanged = 0;
		g_RSSKBaseLightmapHeightFogFOWState.technique = selected.Index;
		g_RSSKBaseLightmapHeightFogFOWState.passes = selected.Passes;
		//Original54348F..5434AF publishes the next descriptor without SetTechnique.
	}
	return result;
}
