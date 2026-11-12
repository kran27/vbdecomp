#include "RSBaseLightmapHeightFogFOW.hpp"
#include "FogOfWarTextureMatrix.hpp"
#include "HeightFog.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"
#include "Viewport.hpp"
#include "TextureTransform.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstring>

Gfx_RS_BaseLightmapHeightFogFOWState g_RSBaseLightmapHeightFogFOWState = {0};

namespace
{
	//Exact original effect bytes; split for the supplied VC7 literal limit.
#define GFX_RS_B_LM_HFOG_FOW_NORMAL_SOURCE \
"DWORD\t\tBcop;\nDWORD\t\tBaop;\nDWORD\t\tLMcop;\nDWORD\t\tLMaop;\nDWORD\t\tTFcop;\nDWORD\t\tTFaop;\nDWORD\t\tFOWcop;\nDWORD\t\tFOWaop;\nDWORD\t\talphatest;\nDWORD\t\tbLighting;\nDWORD\t\tbFog;\nDWORD\t\tDiffuseSource;\nMATRIX\t\tfogmatrix;\nTexture\tHFog;\nMATRIX\t\tFOWmatrix;\nTexture\tFOW;\nTechnique ffp\n{\nPass p0\n{\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nTexture[ 2 ]\t\t\t\t= < HFog >;\nTextureTransform[ 2 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 2 ]\t\t\t= CameraSpacePosition | 2;\nTextureTransformFlags[ 2 ]\t= Count2" \
";\nAddressU[ 2 ]\t\t\t\t= Clamp;\nAddressV[ 2 ]\t\t\t\t= Clamp;\nMipfilter[ 2 ]\t\t\t\t= None;\nColorOp[ 2 ]\t\t\t\t= BlendTextureAlpha;\nColorArg1[ 2 ]\t\t\t\t= Texture;\nColorArg2[ 2 ]\t\t\t\t= Current;\nAlphaOp[ 2 ]\t\t\t\t= SelectArg2;\nAlphaArg1[ 2 ]\t\t\t\t= Texture;\nAlphaArg2[ 2 ]\t\t\t\t= Current;\nTexture[ 3 ]\t\t\t\t= < FOW >;\nTextureTransform[ 3 ]\t\t= < FOWmatrix >;\nTexCoordIndex[ 3 ]\t\t\t= CameraSpacePosition | 3;\nTextureTransformFlags[ 3 ]\t= Count2;\nAddressU[ 3 ]\t\t\t\t= Clamp;\nAddressV[ 3 ]\t\t\t\t= Clamp;\nMipfilter[ 3 ]\t\t\t\t= None;\nColorOp[ 3 ]\t\t\t\t= < FOWcop >;\nColorArg1[ 3 ]\t\t\t\t= Current;\nColorArg2[ 3 ]\t\t\t\t= Texture;\nAlphaOp[ 3 ]\t\t\t\t= < FOWaop >;\nAlphaArg1[ 3 ]\t\t\t\t= Current;\nAlphaArg2[ 3 ]\t\t\t\t= Texture;\nColorOp[ 4 ]\t\t\t\t= Disable;\nAlphaOp[ 4 ]\t\t\t\t= Disable;\n}\n}\nTechnique ffp_2pass\n{\nPass p0\n{\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]" \
"\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nTexture[ 2 ]\t\t\t\t= < HFog >;\nTextureTransform[ 2 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 2 ]\t\t\t= CameraSpacePosition | 2;\nTextureTransformFlags[ 2 ]\t= Count2;\nAddressU[ 2 ]\t\t\t\t= Clamp;\nAddressV[ 2 ]\t\t\t\t= Clamp;\nMipfilter[ 2 ]\t\t\t\t= None;\nColorOp[ 2 ]\t\t\t\t= BlendTextureAlpha;\nColorArg1[ 2 ]\t\t\t\t= Texture;\nColorArg2[ 2 ]\t\t\t\t= Current;\nAlphaOp[ 2 ]\t\t\t\t= SelectArg2;\nAlphaArg1[ 2 ]\t\t\t\t= Texture;\nAlphaArg2[ 2 ]\t\t\t\t= Current;\nColorOp[ 3 ]\t\t\t\t= Disable;\nAlphaOp[ 3 ]\t\t\t\t= Disable;\n}\nPass p1\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= DestColor;\nDestBlend\t\t\t\t\t= Zero;\nBlendOp\t\t\t\t\t= Add;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0" \
" ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nTexture[ 1 ]\t\t\t\t= < FOW >;\nTextureTransform[ 1 ]\t\t= < FOWmatrix >;\nTexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\nTextureTransformFlags[ 1 ]\t= Count2;\nAddressU[ 1 ]\t\t\t\t= Clamp;\nAddressV[ 1 ]\t\t\t\t= Clamp;\nMipfilter[ 1 ]\t\t\t\t= None;\nColorOp[ 1 ]\t\t\t\t= SelectArg2;\nColorArg1[ 1 ]\t\t\t\t= Current;\nColorArg2[ 1 ]\t\t\t\t= Texture;\nAlphaOp[ 1 ]\t\t\t\t= SelectArg1;\nAlphaArg1[ 1 ]\t\t\t\t= Current;\nAlphaArg2[ 1 ]\t\t\t\t= Texture;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\n}\nTechnique ffp_3pass\n{\nPass p0\n{\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\n" \
"AlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\n}\nPass p1\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= SrcAlpha;\nDestBlend\t\t\t\t\t= InvSrcAlpha;\nBlendOp\t\t\t\t\t= Add;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nTexture[ 1 ]\t\t\t\t= < HFog >;\nTextureTransform[ 1 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\nTextureTransformFlags[ 1 ]\t= Count2;\nAddressU[ 1 ]\t\t\t\t= Clamp;\nAddressV[ 1 ]\t\t\t\t= Clamp;\nMipfilter[ 1 ]\t\t\t\t= None;\nColorOp[ 1 ]\t\t\t\t= SelectArg1;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= Modulate;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\n" \
"Pass p2\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= DestColor;\nDestBlend\t\t\t\t\t= Zero;\nBlendOp\t\t\t\t\t= Add;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nTexture[ 1 ]\t\t\t\t= < FOW >;\nTextureTransform[ 1 ]\t\t= < FOWmatrix >;\nTexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\nTextureTransformFlags[ 1 ]\t= Count2;\nAddressU[ 1 ]\t\t\t\t= Clamp;\nAddressV[ 1 ]\t\t\t\t= Clamp;\nMipfilter[ 1 ]\t\t\t\t= None;\nColorOp[ 1 ]\t\t\t\t= SelectArg2;\nColorArg1[ 1 ]\t\t\t\t= Current;\nColorArg2[ 1 ]\t\t\t\t= Texture;\nAlphaOp[ 1 ]\t\t\t\t= SelectArg1;\nAlphaArg1[ 1 ]\t\t\t\t= Current;\nAlphaArg2[ 1 ]\t\t\t\t= Texture;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\n}\n"
	typedef char NormalSourceSize[(sizeof(GFX_RS_B_LM_HFOG_FOW_NORMAL_SOURCE) - 1 == 5492) ? 1 : -1];
#define GFX_RS_B_LM_HFOG_FOW_FACTORED_SOURCE \
"DWORD\t\tBcop;\nDWORD\t\tBaop;\nDWORD\t\tLMcop;\nDWORD\t\tLMaop;\nDWORD\t\tTFcop;\nDWORD\t\tTFaop;\nDWORD\t\tFOWcop;\nDWORD\t\tFOWaop;\nDWORD\t\talphatest;\nDWORD\t\tbLighting;\nDWORD\t\tbFog;\nDWORD\t\tDiffuseSource;\nMATRIX\t\tfogmatrix;\nTexture\tHFog;\nMATRIX\t\tFOWmatrix;\nTexture\tFOW;\nTechnique ffp\n{\nPass p0\n{\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= < TFcop >;\nColorArg1[ 2 ]\t\t\t\t= Current;\nColorArg2[ 2 ]\t\t\t\t= Tfactor;\nAlphaOp[ 2 ]\t\t\t\t= < TFaop >;\nAlphaArg1[ 2 ]\t\t\t\t= Current;\nAlphaA" \
"rg2[ 2 ]\t\t\t\t= Tfactor;\nTexture[ 3 ]\t\t\t\t= < HFog >;\nTextureTransform[ 3 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 3 ]\t\t\t= CameraSpacePosition | 3;\nTextureTransformFlags[ 3 ]\t= Count2;\nAddressU[ 3 ]\t\t\t\t= Clamp;\nAddressV[ 3 ]\t\t\t\t= Clamp;\nMipfilter[ 3 ]\t\t\t\t= None;\nColorOp[ 3 ]\t\t\t\t= BlendTextureAlpha;\nColorArg1[ 3 ]\t\t\t\t= Texture;\nColorArg2[ 3 ]\t\t\t\t= Current;\nAlphaOp[ 3 ]\t\t\t\t= SelectArg2;\nAlphaArg1[ 3 ]\t\t\t\t= Texture;\nAlphaArg2[ 3 ]\t\t\t\t= Current;\nTexture[ 4 ]\t\t\t\t= < FOW >;\nTextureTransform[ 4 ]\t\t= < FOWmatrix >;\nTexCoordIndex[ 4 ]\t\t\t= CameraSpacePosition | 4;\nTextureTransformFlags[ 4 ]\t= Count2;\nAddressU[ 4 ]\t\t\t\t= Clamp;\nAddressV[ 4 ]\t\t\t\t= Clamp;\nMipfilter[ 4 ]\t\t\t\t= None;\nColorOp[ 4 ]\t\t\t\t= < FOWcop >;\nColorArg1[ 4 ]\t\t\t\t= Current;\nColorArg2[ 4 ]\t\t\t\t= Texture;\nAlphaOp[ 4 ]\t\t\t\t= < FOWaop >;\nAlphaArg1[ 4 ]\t\t\t\t= Current;\nAlphaArg2[ 4 ]\t\t\t\t= Texture;\nColorOp[ 5 ]\t\t\t\t= Disable;\nAlphaOp[ 5 ]\t\t\t\t= Disable;\n}\n}\nTechnique ffp_2pass\n{\nPass p0\n{\nAlphaTestEnable\t\t" \
"\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= < TFcop >;\nColorArg1[ 2 ]\t\t\t\t= Current;\nColorArg2[ 2 ]\t\t\t\t= Tfactor;\nAlphaOp[ 2 ]\t\t\t\t= < TFaop >;\nAlphaArg1[ 2 ]\t\t\t\t= Current;\nAlphaArg2[ 2 ]\t\t\t\t= Tfactor;\nTexture[ 3 ]\t\t\t\t= < HFog >;\nTextureTransform[ 3 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 3 ]\t\t\t= CameraSpacePosition | 3;\nTextureTransformFlags[ 3 ]\t= Count2;\nAddressU[ 3 ]\t\t\t\t= Clamp;\nAddressV[ 3 ]\t\t\t\t= Clamp;\nMipfilter[ 3 ]\t\t\t\t= None;\nColorOp[ 3 ]\t\t\t\t= BlendTextureAlpha;\n" \
"ColorArg1[ 3 ]\t\t\t\t= Texture;\nColorArg2[ 3 ]\t\t\t\t= Current;\nAlphaOp[ 3 ]\t\t\t\t= SelectArg2;\nAlphaArg1[ 3 ]\t\t\t\t= Texture;\nAlphaArg2[ 3 ]\t\t\t\t= Current;\nColorOp[ 4 ]\t\t\t\t= Disable;\nAlphaOp[ 4 ]\t\t\t\t= Disable;\n}\nPass\tp1\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= DestColor;\nDestBlend\t\t\t\t\t= Zero;\nBlendOp\t\t\t\t\t= Add;\nTexture[ 0 ]\t\t\t\t= < FOW >;\nTextureTransform[ 0 ]\t\t= < FOWmatrix >;\nTexCoordIndex[ 0 ]\t\t\t= CameraSpacePosition | 0;\nTextureTransformFlags[ 0 ]\t= Count2;\nAddressU[ 0 ]\t\t\t\t= Clamp;\nAddressV[ 0 ]\t\t\t\t= Clamp;\nMipfilter[ 0 ]\t\t\t\t= None;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Current;\nColorArg2[ 0 ]\t\t\t\t= Texture;\nAlphaOp[ 0 ]\t\t\t\t= SelectArg1;\nAlphaArg1[ 0 ]\t\t\t\t= Current;\nAlphaArg2[ 0 ]\t\t\t\t= Texture;\nColorOp[ 1 ]\t\t\t\t= Disable;\nAlphaOp[ 1 ]\t\t\t\t= Disable;\n}\n}\nTechnique ffp_3pass\n{\nPass p0\n{\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMateri" \
"alSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= < TFcop >;\nColorArg1[ 2 ]\t\t\t\t= Current;\nColorArg2[ 2 ]\t\t\t\t= Tfactor;\nAlphaOp[ 2 ]\t\t\t\t= < TFaop >;\nAlphaArg1[ 2 ]\t\t\t\t= Current;\nAlphaArg2[ 2 ]\t\t\t\t= Tfactor;\nColorOp[ 3 ]\t\t\t\t= Disable;\nAlphaOp[ 3 ]\t\t\t\t= Disable;\n}\nPass p1\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= SrcAlpha;\nDestBlend\t\t\t\t\t= InvSrcAlpha;\nBlendOp\t\t\t\t\t= Add;\nTexture[ 0 ]\t\t\t\t= < HFog >;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nTextureTransform[ 1 ]\t\t= < fogm" \
"atrix >;\nTexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\nTextureTransformFlags[ 1 ]\t= Count2;\nAddressU[ 1 ]\t\t\t\t= Clamp;\nAddressV[ 1 ]\t\t\t\t= Clamp;\nMipfilter[ 1 ]\t\t\t\t= None;\nColorOp[ 1 ]\t\t\t\t= SelectArg1;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= Modulate;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\nPass p2\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= DestColor;\nDestBlend\t\t\t\t\t= Zero;\nBlendOp\t\t\t\t\t= Add;\nTexture[ 0 ]\t\t\t\t= < FOW >;\nTextureTransform[ 0 ]\t\t= < FOWmatrix >;\nTexCoordIndex[ 0 ]\t\t\t= CameraSpacePosition | 0;\nTextureTransformFlags[ 0 ]\t= Count2;\nAddressU[ 0 ]\t\t\t\t= Clamp;\nAddressV[ 0 ]\t\t\t\t= Clamp;\nMipfilter[ 0 ]\t\t\t\t= None;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Current;\nColorArg2[ 0 ]\t\t\t\t= Texture;\nAlphaOp[ 0 ]\t\t\t\t= SelectArg1;\nAlphaArg1[ 0 ]\t\t\t\t= Current;\nAlphaArg2[ 0 ]\t\t\t\t= Texture;\nColorOp[ 1 ]\t\t\t\t= Disable;\nAlphaOp[ 1 " \
"]\t\t\t\t= Disable;\n}\n}\nTechnique ffp_4pass\n{\nPass p0\n{\nAlphaTestEnable\t\t\t= < alphatest >;\nLighting\t\t\t\t\t= < bLighting >;\nFogEnable\t\t\t\t\t= < bFog >;\nDiffuseMaterialSource\t\t= < DiffuseSource >;\nAmbientMaterialSource\t\t= < DiffuseSource >;\nColorOp[ 0 ]\t\t\t\t= < Bcop >;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nColorOp[ 1 ]\t\t\t\t= < LMcop >;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= < LMaop >;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 3 ]\t\t\t\t= Disable;\nAlphaOp[ 3 ]\t\t\t\t= Disable;\n}\nPass p1\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= DestColor;\nDestBlend\t\t\t\t\t= SrcColor;\nBlendOp\t\t\t\t\t= Add;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Current;\nColorArg2[ 0 ]\t\t\t\t= Tfactor;\nAlphaOp[ 0 ]\t\t\t\t= < TFaop >;\nAlphaArg1[ 0 ]\t\t\t\t= Current;\nAlphaArg2[ 0 ]\t\t\t\t= Tfactor;\nColorOp[ 1 ]\t\t\t\t= Disable;\nAlphaOp" \
"[ 1 ]\t\t\t\t= Disable;\n}\nPass p2\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= SrcAlpha;\nDestBlend\t\t\t\t\t= InvSrcAlpha;\nBlendOp\t\t\t\t\t= Add;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Texture;\nColorArg2[ 0 ]\t\t\t\t= Diffuse;\nAlphaOp[ 0 ]\t\t\t\t= < Baop >;\nAlphaArg1[ 0 ]\t\t\t\t= Texture;\nAlphaArg2[ 0 ]\t\t\t\t= Diffuse;\nTexture[ 1 ]\t\t\t\t= < HFog >;\nTextureTransform[ 1 ]\t\t= < fogmatrix >;\nTexCoordIndex[ 1 ]\t\t\t= CameraSpacePosition | 1;\nTextureTransformFlags[ 1 ]\t= Count2;\nAddressU[ 1 ]\t\t\t\t= Clamp;\nAddressV[ 1 ]\t\t\t\t= Clamp;\nMipfilter[ 1 ]\t\t\t\t= None;\nColorOp[ 1 ]\t\t\t\t= SelectArg1;\nColorArg1[ 1 ]\t\t\t\t= Texture;\nColorArg2[ 1 ]\t\t\t\t= Current;\nAlphaOp[ 1 ]\t\t\t\t= Modulate;\nAlphaArg1[ 1 ]\t\t\t\t= Texture;\nAlphaArg2[ 1 ]\t\t\t\t= Current;\nColorOp[ 2 ]\t\t\t\t= Disable;\nAlphaOp[ 2 ]\t\t\t\t= Disable;\n}\nPass p3\n{\nAlphaBlendEnable\t\t\t= TRUE;\nSrcBlend\t\t\t\t\t= DestColor;\nDestBlend\t\t\t\t\t= Zero;\nBlendOp\t\t\t\t\t= Add;\nTexture[ 0 ]\t\t\t\t= < FOW >;\nTextureTransform[ 0 ]\t\t= < FOWmatrix >;\nTexCoordInde" \
"x[ 0 ]\t\t\t= CameraSpacePosition | 0;\nTextureTransformFlags[ 0 ]\t= Count2;\nAddressU[ 0 ]\t\t\t\t= Clamp;\nAddressV[ 0 ]\t\t\t\t= Clamp;\nMipfilter[ 0 ]\t\t\t\t= None;\nColorOp[ 0 ]\t\t\t\t= SelectArg2;\nColorArg1[ 0 ]\t\t\t\t= Current;\nColorArg2[ 0 ]\t\t\t\t= Texture;\nAlphaOp[ 0 ]\t\t\t\t= SelectArg1;\nAlphaArg1[ 0 ]\t\t\t\t= Current;\nAlphaArg2[ 0 ]\t\t\t\t= Texture;\nColorOp[ 1 ]\t\t\t\t= Disable;\nAlphaOp[ 1 ]\t\t\t\t= Disable;\n}\n}\n"
	typedef char FactoredSourceSize[(sizeof(GFX_RS_B_LM_HFOG_FOW_FACTORED_SOURCE) - 1 == 7985) ? 1 : -1];

	HRESULT UploadFogConstants(const D3DMATERIAL8& material,
		const D3DXVECTOR3& position, float thickness)
	{
		const float constants[4] = { 0.0f, 0.5f, 1.0f, 2.0f };
		g_D3D.device->SetVertexShaderConstant(0, constants, 1);
		BYTE red, green, blue;
		g_GfxSystem->GetAmbientRGB(&red, &green, &blue);
		const float ambient[4] = { red * 0.0039215689f,
			green * 0.0039215689f, blue * 0.0039215689f, 1.0f };
		g_D3D.device->SetVertexShaderConstant(1, ambient, 1);
		g_GfxSystem->CurrentViewport();
		Gfx_Viewport* viewport = g_GfxSystem->CurrentViewport();
		const float range[4] = { viewport->fogEnd,
			float(1.0 / (double(viewport->fogEnd) - viewport->fogStart)), 0.0f, 0.0f };
		g_D3D.device->SetVertexShaderConstant(2, range, 1);
		g_D3D.device->SetVertexShaderConstant(3, &material.Diffuse, 1);
		g_D3D.device->SetVertexShaderConstant(4, &material.Emissive, 1);
		const float fogPosition[4] = { position.x, position.y, position.z, 0.0f };
		g_D3D.device->SetVertexShaderConstant(40, fogPosition, 1);
		const float scale[4] = { 1.0f, float(1.0 / double(thickness)),
			0.0066666668f, 0.0034482759f };
		return g_D3D.device->SetVertexShaderConstant(41, scale, 1);
	}
}

Gfx_RS_B_LM_HFog_FOW::Gfx_RS_B_LM_HFog_FOW(DWORD color, DWORD alpha,
	DWORD lightmapColor, DWORD lightmapAlpha, DWORD test)
: effect(NULL), colorOperation(color), alphaOperation(alpha),
	lightmapColorOperation(lightmapColor), lightmapAlphaOperation(lightmapAlpha),
	alphaTest(test)
{
}

Gfx_RS_B_LM_HFog_FOW::~Gfx_RS_B_LM_HFog_FOW()
{
	Release();
}

HRESULT Gfx_RS_B_LM_HFog_FOW::Initialize()
{
	HRESULT result = D3DERR_INVALIDCALL;
	++g_RSBaseLightmapHeightFogFOWState.users;
	ID3DXBuffer* errors;
	if (!g_RSBaseLightmapHeightFogFOWState.normal)
	{
		result = D3DXCreateEffect(g_D3D.device, GFX_RS_B_LM_HFOG_FOW_NORMAL_SOURCE, sizeof(GFX_RS_B_LM_HFOG_FOW_NORMAL_SOURCE) - 1,
			&g_RSBaseLightmapHeightFogFOWState.normal, &errors);
		if (SUCCEEDED(result))
		{
			g_RSBaseLightmapHeightFogFOWState.normal->GetDesc(&g_RSBaseLightmapHeightFogFOWState.description);
			g_RSBaseLightmapHeightFogFOWState.normal->SetTechnique(0);
			D3DXMATRIX identity = g_D3D.identity;
			g_RSBaseLightmapHeightFogFOWState.normal->SetDword("alphatest", 0);
			g_RSBaseLightmapHeightFogFOWState.normal->SetDword("bLighting", 0);
			g_RSBaseLightmapHeightFogFOWState.normal->SetDword("bFog", 0);
			g_RSBaseLightmapHeightFogFOWState.normal->SetDword("Bcop", colorOperation);
			g_RSBaseLightmapHeightFogFOWState.normal->SetDword("Baop", alphaOperation);
			g_RSBaseLightmapHeightFogFOWState.normal->SetDword("LMcop", lightmapColorOperation);
			g_RSBaseLightmapHeightFogFOWState.normal->SetDword("LMaop", lightmapAlphaOperation);
			g_RSBaseLightmapHeightFogFOWState.normal->SetDword("TFcop", 2);
			g_RSBaseLightmapHeightFogFOWState.normal->SetDword("TFaop", 2);
			g_RSBaseLightmapHeightFogFOWState.normal->SetDword("FOWcop", 2);
			g_RSBaseLightmapHeightFogFOWState.normal->SetDword("FOWaop", 2);
			g_RSBaseLightmapHeightFogFOWState.normal->SetDword("DiffuseSource", 1);
			g_RSBaseLightmapHeightFogFOWState.normal->SetMatrix("fogmatrix", &identity);
			g_RSBaseLightmapHeightFogFOWState.normal->SetTexture("HFog", NULL);
			g_RSBaseLightmapHeightFogFOWState.normal->SetTexture("FOW", NULL);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSBaseLightmapHeightFogFOWState.normal->FindNextValidTechnique(g_RSBaseLightmapHeightFogFOWState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSBaseLightmapHeightFogFOWState.technique = selected.Index;
				result = g_RSBaseLightmapHeightFogFOWState.normal->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h') g_RSBaseLightmapHeightFogFOWState.usesShader = 1;
				g_RSBaseLightmapHeightFogFOWState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_B_LM_HFog_FOW::Create - Failed to find valid technique (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_B_LM_HFog_FOW - Failed to compile\n");
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors)
		{
			errors->Release();
			errors = NULL;
		}
	}
	if (!g_RSBaseLightmapHeightFogFOWState.factored)
	{
		result = D3DXCreateEffect(g_D3D.device, GFX_RS_B_LM_HFOG_FOW_FACTORED_SOURCE, sizeof(GFX_RS_B_LM_HFOG_FOW_FACTORED_SOURCE) - 1,
			&g_RSBaseLightmapHeightFogFOWState.factored, &errors);
		if (SUCCEEDED(result))
		{
			g_RSBaseLightmapHeightFogFOWState.factored->GetDesc(&g_RSBaseLightmapHeightFogFOWState.description);
			g_RSBaseLightmapHeightFogFOWState.factored->SetTechnique(0);
			D3DXMATRIX identity = g_D3D.identity;
			g_RSBaseLightmapHeightFogFOWState.factored->SetDword("alphatest", 0);
			g_RSBaseLightmapHeightFogFOWState.factored->SetDword("bLighting", 0);
			g_RSBaseLightmapHeightFogFOWState.factored->SetDword("bFog", 0);
			g_RSBaseLightmapHeightFogFOWState.factored->SetDword("Bcop", colorOperation);
			g_RSBaseLightmapHeightFogFOWState.factored->SetDword("Baop", alphaOperation);
			g_RSBaseLightmapHeightFogFOWState.factored->SetDword("LMcop", lightmapColorOperation);
			g_RSBaseLightmapHeightFogFOWState.factored->SetDword("LMaop", lightmapAlphaOperation);
			g_RSBaseLightmapHeightFogFOWState.factored->SetDword("TFcop", 2);
			g_RSBaseLightmapHeightFogFOWState.factored->SetDword("TFaop", 2);
			g_RSBaseLightmapHeightFogFOWState.factored->SetDword("FOWcop", 2);
			g_RSBaseLightmapHeightFogFOWState.factored->SetDword("FOWaop", 2);
			g_RSBaseLightmapHeightFogFOWState.factored->SetDword("DiffuseSource", 1);
			g_RSBaseLightmapHeightFogFOWState.factored->SetMatrix("fogmatrix", &identity);
			g_RSBaseLightmapHeightFogFOWState.factored->SetTexture("HFog", NULL);
			g_RSBaseLightmapHeightFogFOWState.factored->SetTexture("FOW", NULL);
			D3DXTECHNIQUE_DESC selected;
			result = g_RSBaseLightmapHeightFogFOWState.factored->FindNextValidTechnique(g_RSBaseLightmapHeightFogFOWState.technique, &selected);
			if (SUCCEEDED(result))
			{
				g_RSBaseLightmapHeightFogFOWState.technique = selected.Index;
				result = g_RSBaseLightmapHeightFogFOWState.factored->SetTechnique(selected.Index);
				if (selected.Name[0] == 's' && selected.Name[1] == 'h') g_RSBaseLightmapHeightFogFOWState.usesShader = 1;
				if (g_RSBaseLightmapHeightFogFOWState.passes < selected.Passes) g_RSBaseLightmapHeightFogFOWState.passes = selected.Passes;
			}
			else
				JE::FatalError("Gfx_RS_B_LM_HFog_FOW::Create - Failed to find valid technique (%x)\n", result);
		}
		else
		{
			JE::LogDebugString("Gfx_RS_B_LM_HFog_FOW - Failed to compile\n");
			JE::FatalError(static_cast<const char*>(errors->GetBufferPointer()));
		}
		if (errors)
		{
			errors->Release();
		}
	}
	return result;
}

void Gfx_RS_B_LM_HFog_FOW::Release()
{
	if (g_RSBaseLightmapHeightFogFOWState.users)
	{
		--g_RSBaseLightmapHeightFogFOWState.users;
		if (!g_RSBaseLightmapHeightFogFOWState.users)
		{
			if (g_RSBaseLightmapHeightFogFOWState.normal)
			{
				g_RSBaseLightmapHeightFogFOWState.normal->Release();
				g_RSBaseLightmapHeightFogFOWState.normal = NULL;
			}
			if (g_RSBaseLightmapHeightFogFOWState.factored)
			{
				g_RSBaseLightmapHeightFogFOWState.factored->Release();
				g_RSBaseLightmapHeightFogFOWState.factored = NULL;
			}
		}
	}
}

HRESULT Gfx_RS_B_LM_HFog_FOW::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseLightmapHeightFogFOWState.normal) result = g_RSBaseLightmapHeightFogFOWState.normal->OnLostDevice();
	if (g_RSBaseLightmapHeightFogFOWState.factored) result = g_RSBaseLightmapHeightFogFOWState.factored->OnLostDevice();
	return result;
}

HRESULT Gfx_RS_B_LM_HFog_FOW::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseLightmapHeightFogFOWState.normal) result = g_RSBaseLightmapHeightFogFOWState.normal->OnResetDevice();
	if (g_RSBaseLightmapHeightFogFOWState.factored) result = g_RSBaseLightmapHeightFogFOWState.factored->OnResetDevice();
	return result;
}

DWORD Gfx_RS_B_LM_HFog_FOW::TechniqueCount() { return g_RSBaseLightmapHeightFogFOWState.description.Techniques; }
BYTE Gfx_RS_B_LM_HFog_FOW::UsesVertexShader() { return g_RSBaseLightmapHeightFogFOWState.usesShader; }
DWORD Gfx_RS_B_LM_HFog_FOW::PassCount() { return g_RSBaseLightmapHeightFogFOWState.passes; }

DWORD Gfx_RS_B_LM_HFog_FOW::SelectTechnique(DWORD index)
{
	if (index < g_RSBaseLightmapHeightFogFOWState.description.Techniques &&
		index != reinterpret_cast<DWORD>(g_RSBaseLightmapHeightFogFOWState.technique))
	{
		g_RSBaseLightmapHeightFogFOWState.techniqueChanged = 1;
		g_RSBaseLightmapHeightFogFOWState.technique = reinterpret_cast<LPCSTR>(index);
	}
	return index;
}

DWORD Gfx_RS_B_LM_HFog_FOW::NextTechnique()
{
	DWORD next = reinterpret_cast<DWORD>(g_RSBaseLightmapHeightFogFOWState.technique) + 1;
	if (next >= g_RSBaseLightmapHeightFogFOWState.description.Techniques) next = 0;
	return SelectTechnique(next);
}

HRESULT Gfx_RS_B_LM_HFog_FOW::Begin(UINT* passes) { return effect->Begin(passes, 0); }
HRESULT Gfx_RS_B_LM_HFog_FOW::End() { return effect->End(); }

HRESULT Gfx_RS_B_LM_HFog_FOW::Pass(UINT pass)
{
	g_RSBaseLightmapHeightFogFOWState.currentPass = pass;
	return effect->Pass(pass);
}

HRESULT Gfx_RS_B_LM_HFog_FOW::SetMaterial(Gfx_Material* material, DWORD fogBits)
{
	BYTE useRGB = 0, useAlpha = 0;
	DWORD color;
	material->GetColor(useRGB, useAlpha, color);
	effect = useRGB || useAlpha ? g_RSBaseLightmapHeightFogFOWState.factored : g_RSBaseLightmapHeightFogFOWState.normal;
	if (!effect)
	{
		JE::LogDebugString("Gfx_RS_B_LM_HFog_FOW::SetMaterial() failed - not created yet.");
		return D3DERR_INVALIDCALL;
	}
	Gfx_HeightFog* fog = reinterpret_cast<Gfx_HeightFog*>(fogBits);
	if (!fog)
	{
		JE::LogDebugString("Gfx_RS_B_LM_HFog_FOW: No HeightFog object specified!");
		return D3DERR_INVALIDCALL;
	}
	const DWORD baseHandle = material->textureHandles[0];
	if (baseHandle)
	{
		Gfx_Texture* texture = g_TextureManager->Lookup(baseHandle);
		if (texture) texture->Bind(0, 0);
		else
		{
			g_D3D.SetTexture(0, NULL);
			const std::string name = texture->GetDiagnosticName();
			JE::LogDebugString("Gfx_RS_B_LM_HFog_FOW: Cannot find Base Texture %s", name.c_str());
		}
	}
	else
	{
		g_D3D.SetTexture(0, NULL);
		JE::LogDebugString("Gfx_RS_B_LM_HFog_FOW: NULL Base Texture handle\n");
	}
	const DWORD lightmapHandle = material->textureHandles[1];
	if (lightmapHandle)
	{
		Gfx_Texture* texture = g_TextureManager->Lookup(lightmapHandle);
		if (texture) texture->Bind(1, 0);
		else
		{
			g_D3D.SetTexture(1, NULL);
			const std::string name = texture->GetDiagnosticName();
			JE::LogDebugString("Gfx_RS_B_LM_HFog_FOW: Cannot find Lightmap %s", name.c_str());
		}
	}
	else g_D3D.SetTexture(1, NULL);
	if (fog->value44)
	{
		Gfx_Texture* texture = g_TextureManager->Lookup(fog->value44);
		if (texture)
		{
			IDirect3DTexture8* frame = texture->frameCount > 0 ? texture->frames[0].texture : NULL;
			effect->SetTexture("HFog", frame);
		}
		else
		{
			effect->SetTexture("HFog", NULL);
			JE::LogDebugString("Gfx_RS_B_LM_HFog_FOW: Height Fog map not found!");
		}
	}
	else
	{
		effect->SetTexture("HFog", NULL);
		JE::LogDebugString("Gfx_RS_B_LM_HFog_FOW: Height Fog map not created!");
	}
	IDirect3DTexture8* fowTexture = g_GfxSystem->FogOfWarRenderTexture();
	effect->SetTexture("FOW", fowTexture);
	effect->SetDword("FOWcop", fowTexture ? 4 : 1);
	effect->SetDword("FOWaop", fowTexture ? 2 : 1);
	effect->SetDword("alphatest", alphaTest);
	BOOL lighting = material->flagBC ? g_GfxSystem->Lighting() != 0 : FALSE;
	effect->SetDword("bLighting", lighting);
	BOOL distanceFog = material->flagBD ? g_GfxSystem->CurrentViewport()->fogEnabled != 0 : FALSE;
	effect->SetDword("bFog", distanceFog);
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
	D3DXMATRIX fowMatrix;
	Gfx_BuildFogOfWarTextureMatrix(fowMatrix);
	effect->SetMatrix("FOWmatrix", &fowMatrix);
	const float seconds = g_GfxSystem->TimeSeconds();
	float offset, thickness;
	fog->Evaluate(seconds, &offset, &thickness);
	const D3DXVECTOR3 position(fog->transform._41, fog->transform._42, fog->transform._43);
	if (g_RSBaseLightmapHeightFogFOWState.usesShader)
	{
		result = UploadFogConstants(values, position, thickness);
	}
	else
	{
		D3DXMATRIX map = g_D3D.identity;
		D3DXMATRIX inverseView;
		g_GfxSystem->Camera()->GetInverseViewMatrix(inverseView);
		const double inverse = 1.0 / double(thickness);
		map._11 = 0.0f;
		map._33 = 0.0f;
		map._22 = float(inverse);
		map._42 = float(-((double(position.y) + offset) * inverse));
		D3DXMATRIX product;
		D3DXMatrixMultiply(&product, &inverseView, &map);
		D3DXMATRIX fogMatrix = product;
		effect->SetMatrix("fogmatrix", &fogMatrix);
	}
	const DWORD flags = material->values140[0];
	if (flags)
		Gfx_SetTextureTransform(0, static_cast<BYTE>(flags), &material->textureParameters[0], seconds);
	else g_D3D.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	if (g_RSBaseLightmapHeightFogFOWState.techniqueChanged)
	{
		D3DXTECHNIQUE_DESC selected;
		result = effect->FindNextValidTechnique(g_RSBaseLightmapHeightFogFOWState.technique, &selected);
		if (FAILED(result))
			JE::FatalError("Gfx_RS_B_LM_HFog_FOW::SetMaterial - Failed to find valid technique (%x)\n", result);
		g_RSBaseLightmapHeightFogFOWState.techniqueChanged = 0;
		g_RSBaseLightmapHeightFogFOWState.technique = selected.Index;
		g_RSBaseLightmapHeightFogFOWState.passes = selected.Passes;
	}
	return result;
}
