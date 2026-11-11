#include "RenderStyleRegistry.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "RSBase.hpp"
#include "RSBaseLightmap.hpp"
#include "RSBaseEmissive.hpp"
#include "RSBaseHeightFog.hpp"
#include "RSBaseLightmapHeightFog.hpp"
#include "RSBaseFOW.hpp"
#include "RSBaseLightmapFogOfWar.hpp"
#include "RSBaseHeightFogFOW.hpp"
#include "RSBaseLightmapHeightFogFOW.hpp"
#include "RSExpand.hpp"
#include "RSSKBaseLightmap.hpp"
#include "RSSKBaseLightmapHeightFog.hpp"
#include "RSSKBaseLightmapFOW.hpp"
#include "RSSKBaseLightmapHeightFogFOW.hpp"

//One production storage owner for the original loader-zero states. Isolated
//unit fixtures may define their own storage without linking this registry.
Gfx_RS_BaseState g_RSBaseState;
Gfx_RS_B_LMState g_RSBaseLightmapState;
Gfx_RS_SK_B_LMState g_RSSKBaseLightmapState;

namespace
{
	extern Gfx_RS_Base style70B790; //0x0070B790.
	extern Gfx_RS_Base style70BB3C; //0x0070BB3C.
	extern Gfx_RS_Base style70B850; //0x0070B850.
	extern Gfx_RS_Base style70B780; //0x0070B780.
	extern Gfx_RS_Base style70B860; //0x0070B860.
	extern Gfx_RS_Base style70B5E4; //0x0070B5E4.
	extern Gfx_RS_Base style70B8B4; //0x0070B8B4.
	extern Gfx_RS_Base style70B50C; //0x0070B50C.
	extern Gfx_RS_Base style70B8A4; //0x0070B8A4.
	extern Gfx_RS_Base style70BAA4; //0x0070BAA4.
	extern Gfx_RS_B_LM style70B694; //0x0070B694.
	extern Gfx_RS_B_LM style70B9C8; //0x0070B9C8.
	extern Gfx_RS_B_LM style70B60C; //0x0070B60C.
	extern Gfx_RS_B_LM style70B8FC; //0x0070B8FC.
	extern Gfx_RS_B_LM style70B8C4; //0x0070B8C4.
	extern Gfx_RS_B_LM style70BA2C; //0x0070BA2C.
	extern Gfx_RS_B_LM style70B39C; //0x0070B39C.
	extern Gfx_RS_B_LM style70B830; //0x0070B830.
	extern Gfx_RS_B_LM style70B5C8; //0x0070B5C8.
	extern Gfx_RS_B_EM style70B934; //0x0070B934.
	extern Gfx_RS_B_EM style70B628; //0x0070B628.
	extern Gfx_RS_B_EM style70B944; //0x0070B944.
	extern Gfx_RS_B_EM style70B760; //0x0070B760.
	extern Gfx_RS_B_EM style70B6F4; //0x0070B6F4.
	extern Gfx_RS_B_EM style70B6B0; //0x0070B6B0.
	extern Gfx_RS_B_EM style70B770; //0x0070B770.
	extern Gfx_RS_B_EM style70B638; //0x0070B638.
	extern Gfx_RS_Base_HeightFog style70B568; //0x0070B568.
	extern Gfx_RS_Base_HeightFog style70B418; //0x0070B418.
	extern Gfx_RS_Base_HeightFog style70B704; //0x0070B704.
	extern Gfx_RS_Base_HeightFog style70B730; //0x0070B730.
	extern Gfx_RS_Base_HeightFog style70B988; //0x0070B988.
	extern Gfx_RS_Base_HeightFog style70B3D4; //0x0070B3D4.
	extern Gfx_RS_Base_HeightFog style70B740; //0x0070B740.
	extern Gfx_RS_Base_HeightFog style70B38C; //0x0070B38C.
	extern Gfx_RS_Base_HeightFog style70B334; //0x0070B334.
	extern Gfx_RS_Base_HeightFog style70B344; //0x0070B344.
	extern Gfx_RS_B_LM_HeightFog style70B3B8; //0x0070B3B8.
	extern Gfx_RS_B_LM_HeightFog style70BA78; //0x0070BA78.
	extern Gfx_RS_B_LM_HeightFog style70B318; //0x0070B318.
	extern Gfx_RS_B_LM_HeightFog style70B530; //0x0070B530.
	extern Gfx_RS_B_LM_HeightFog style70B4C0; //0x0070B4C0.
	extern Gfx_RS_B_LM_HeightFog style70BAC4; //0x0070BAC4.
	extern Gfx_RS_B_LM_HeightFog style70B8E0; //0x0070B8E0.
	extern Gfx_RS_B_LM_HeightFog style70B474; //0x0070B474.
	extern Gfx_RS_B_LM_HeightFog style70B648; //0x0070B648.
	extern Gfx_RS_Base_FOW style70B438; //0x0070B438.
	extern Gfx_RS_Base_FOW style70BA00; //0x0070BA00.
	extern Gfx_RS_Base_FOW style70B51C; //0x0070B51C.
	extern Gfx_RS_Base_FOW style70BB28; //0x0070BB28.
	extern Gfx_RS_Base_FOW style70B490; //0x0070B490.
	extern Gfx_RS_Base_FOW style70BA64; //0x0070BA64.
	extern Gfx_RS_Base_FOW style70B6D0; //0x0070B6D0.
	extern Gfx_RS_Base_FOW style70B664; //0x0070B664.
	extern Gfx_RS_Base_FOW style70B4DC; //0x0070B4DC.
	extern Gfx_RS_Base_FOW style70B7E8; //0x0070B7E8.
	extern Gfx_RS_B_LM_FOW style70B2E8; //0x0070B2E8.
	extern Gfx_RS_B_LM_FOW style70B3E4; //0x0070B3E4.
	extern Gfx_RS_B_LM_FOW style70B300; //0x0070B300.
	extern Gfx_RS_B_LM_FOW style70B998; //0x0070B998.
	extern Gfx_RS_B_LM_FOW style70B9B0; //0x0070B9B0.
	extern Gfx_RS_B_LM_FOW style70B578; //0x0070B578.
	extern Gfx_RS_B_LM_FOW style70B44C; //0x0070B44C.
	extern Gfx_RS_B_LM_FOW style70B7FC; //0x0070B7FC.
	extern Gfx_RS_B_LM_FOW style70B970; //0x0070B970.
	extern Gfx_RS_Base_HFog_FOW style70B428; //0x0070B428.
	extern Gfx_RS_Base_HFog_FOW style70BAB4; //0x0070BAB4.
	extern Gfx_RS_Base_HFog_FOW style70BB68; //0x0070BB68.
	extern Gfx_RS_Base_HFog_FOW style70B7D8; //0x0070B7D8.
	extern Gfx_RS_Base_HFog_FOW style70BAE0; //0x0070BAE0.
	extern Gfx_RS_Base_HFog_FOW style70B6C0; //0x0070B6C0.
	extern Gfx_RS_Base_HFog_FOW style70B750; //0x0070B750.
	extern Gfx_RS_Base_HFog_FOW style70B464; //0x0070B464.
	extern Gfx_RS_Base_HFog_FOW style70B6E4; //0x0070B6E4.
	extern Gfx_RS_Base_HFog_FOW style70BA94; //0x0070BA94.
	extern Gfx_RS_B_LM_HFog_FOW style70B7A0; //0x0070B7A0.
	extern Gfx_RS_B_LM_HFog_FOW style70B354; //0x0070B354.
	extern Gfx_RS_B_LM_HFog_FOW style70B870; //0x0070B870.
	extern Gfx_RS_B_LM_HFog_FOW style70B954; //0x0070B954.
	extern Gfx_RS_B_LM_HFog_FOW style70B4F0; //0x0070B4F0.
	extern Gfx_RS_B_LM_HFog_FOW style70B9E4; //0x0070B9E4.
	extern Gfx_RS_B_LM_HFog_FOW style70B7BC; //0x0070B7BC.
	extern Gfx_RS_B_LM_HFog_FOW style70B5AC; //0x0070B5AC.
	extern Gfx_RS_B_LM_HFog_FOW style70BB0C; //0x0070BB0C.
	extern Gfx_RS_Expand style70B84C; //0x0070B84C.
	extern Gfx_RS_SK_B_LM style70B814; //0x0070B814.
	extern Gfx_RS_SK_B_LM style70B4A4; //0x0070B4A4.
	extern Gfx_RS_SK_B_LM style70B54C; //0x0070B54C.
	extern Gfx_RS_SK_B_LM style70BA48; //0x0070BA48.
	extern Gfx_RS_SK_B_LM_HeightFog style70BB4C; //0x0070BB4C.
	extern Gfx_RS_SK_B_LM_HeightFog style70B918; //0x0070B918.
	extern Gfx_RS_SK_B_LM_HeightFog style70B3FC; //0x0070B3FC.
	extern Gfx_RS_SK_B_LM_HeightFog style70BAF0; //0x0070BAF0.
	extern Gfx_RS_SK_B_LM_FOW style70BA14; //0x0070BA14.
	extern Gfx_RS_SK_B_LM_FOW style70B5F4; //0x0070B5F4.
	extern Gfx_RS_SK_B_LM_FOW style70B2D0; //0x0070B2D0.
	extern Gfx_RS_SK_B_LM_FOW style70B88C; //0x0070B88C.
	extern Gfx_RS_SK_B_LM_HFog_FOW style70B678; //0x0070B678.
	extern Gfx_RS_SK_B_LM_HFog_FOW style70B714; //0x0070B714.
	extern Gfx_RS_SK_B_LM_HFog_FOW style70B590; //0x0070B590.
	extern Gfx_RS_SK_B_LM_HFog_FOW style70B370; //0x0070B370.
}

//Original6FBD20..6FBFC7: both empty physical rows are retained.
Gfx_RenderStyle* g_RenderStyleVariants[34][5] =
{
	{&style70B790, &style70B568, &style70B568, &style70B438, &style70B428}, //0,0x006FBD20.
	{&style70B850, &style70B704, &style70B704, &style70B51C, &style70BB68}, //1,0x006FBD34.
	{&style70B780, &style70B730, &style70B730, &style70BB28, &style70B7D8}, //2,0x006FBD48.
	{&style70B934, NULL, NULL, NULL, NULL}, //3,0x006FBD5C.
	{&style70B628, NULL, NULL, NULL, NULL}, //4,0x006FBD70.
	{&style70B694, &style70B3B8, &style70B3B8, &style70B2E8, &style70B7A0}, //5,0x006FBD84.
	{&style70B9C8, &style70BA78, &style70BA78, &style70B3E4, &style70B354}, //6,0x006FBD98.
	{&style70B5C8, &style70B648, &style70B648, &style70B970, &style70BB0C}, //7,0x006FBDAC.
	{&style70B8B4, &style70B740, &style70B740, &style70B6D0, &style70B750}, //8,0x006FBDC0.
	{&style70B50C, &style70B38C, &style70B38C, &style70B664, &style70B464}, //9,0x006FBDD4.
	{&style70B6F4, NULL, NULL, NULL, NULL}, //10,0x006FBDE8.
	{&style70B6B0, NULL, NULL, NULL, NULL}, //11,0x006FBDFC.
	{&style70B8C4, &style70B318, &style70B318, &style70B300, &style70B870}, //12,0x006FBE10.
	{&style70BA2C, &style70B530, &style70B530, &style70B998, &style70B954}, //13,0x006FBE24.
	{&style70BB3C, &style70B418, &style70B418, &style70BA00, &style70BAB4}, //14,0x006FBE38.
	{&style70B860, &style70B988, &style70B988, &style70B490, &style70BAE0}, //15,0x006FBE4C.
	{&style70B5E4, &style70B3D4, &style70B3D4, &style70BA64, &style70B6C0}, //16,0x006FBE60.
	{&style70B944, NULL, NULL, NULL, NULL}, //17,0x006FBE74.
	{&style70B760, NULL, NULL, NULL, NULL}, //18,0x006FBE88.
	{&style70B8A4, &style70B334, &style70B334, &style70B4DC, &style70B6E4}, //19,0x006FBE9C.
	{&style70BAA4, &style70B344, &style70B344, &style70B7E8, &style70BA94}, //20,0x006FBEB0.
	{&style70B770, NULL, NULL, NULL, NULL}, //21,0x006FBEC4.
	{&style70B638, NULL, NULL, NULL, NULL}, //22,0x006FBED8.
	{&style70B60C, &style70B4C0, &style70B4C0, &style70B9B0, &style70B4F0}, //23,0x006FBEEC.
	{&style70B8FC, &style70BAC4, &style70BAC4, &style70B578, &style70B9E4}, //24,0x006FBF00.
	{&style70B39C, &style70B8E0, &style70B8E0, &style70B44C, &style70B7BC}, //25,0x006FBF14.
	{&style70B830, &style70B474, &style70B474, &style70B7FC, &style70B5AC}, //26,0x006FBF28.
	{&style70B84C, NULL, NULL, NULL, NULL}, //27,0x006FBF3C.
	{&style70B814, &style70BB4C, &style70BB4C, &style70BA14, &style70B678}, //28,0x006FBF50.
	{&style70B4A4, &style70B918, &style70B918, &style70B5F4, &style70B714}, //29,0x006FBF64.
	{&style70B54C, &style70B3FC, &style70B3FC, &style70B2D0, &style70B590}, //30,0x006FBF78.
	{&style70BA48, &style70BAF0, &style70BAF0, &style70B88C, &style70B370}, //31,0x006FBF8C.
	{NULL, NULL, NULL, NULL, NULL}, //32,0x006FBFA0.
	{NULL, NULL, NULL, NULL, NULL}, //33,0x006FBFB4.
};

//Original641A20..641E5A constructs all34 controllers before style objects.
namespace { Gfx_RenderStyleController controller708AD4(g_RenderStyleVariants[0]); }
namespace { Gfx_RenderStyleController controller708AA4(g_RenderStyleVariants[1]); }
namespace { Gfx_RenderStyleController controller708C84(g_RenderStyleVariants[2]); }
namespace { Gfx_RenderStyleController controller708CFC(g_RenderStyleVariants[3]); }
namespace { Gfx_RenderStyleController controller708BF4(g_RenderStyleVariants[4]); }
namespace { Gfx_RenderStyleController controller708D2C(g_RenderStyleVariants[5]); }
namespace { Gfx_RenderStyleController controller708C6C(g_RenderStyleVariants[6]); }
namespace { Gfx_RenderStyleController controller708A74(g_RenderStyleVariants[7]); }
namespace { Gfx_RenderStyleController controller708C3C(g_RenderStyleVariants[8]); }
namespace { Gfx_RenderStyleController controller708BC4(g_RenderStyleVariants[9]); }
namespace { Gfx_RenderStyleController controller708AEC(g_RenderStyleVariants[10]); }
namespace { Gfx_RenderStyleController controller708D14(g_RenderStyleVariants[11]); }
namespace { Gfx_RenderStyleController controller708BAC(g_RenderStyleVariants[12]); }
namespace { Gfx_RenderStyleController controller708CCC(g_RenderStyleVariants[13]); }
namespace { Gfx_RenderStyleController controller708C9C(g_RenderStyleVariants[14]); }
namespace { Gfx_RenderStyleController controller708D5C(g_RenderStyleVariants[15]); }
namespace { Gfx_RenderStyleController controller708C24(g_RenderStyleVariants[16]); }
namespace { Gfx_RenderStyleController controller708C0C(g_RenderStyleVariants[17]); }
namespace { Gfx_RenderStyleController controller708B4C(g_RenderStyleVariants[18]); }
namespace { Gfx_RenderStyleController controller708CE4(g_RenderStyleVariants[19]); }
namespace { Gfx_RenderStyleController controller708B04(g_RenderStyleVariants[20]); }
namespace { Gfx_RenderStyleController controller708A5C(g_RenderStyleVariants[21]); }
namespace { Gfx_RenderStyleController controller708B64(g_RenderStyleVariants[22]); }
namespace { Gfx_RenderStyleController controller708B94(g_RenderStyleVariants[23]); }
namespace { Gfx_RenderStyleController controller708CB4(g_RenderStyleVariants[24]); }
namespace { Gfx_RenderStyleController controller708BDC(g_RenderStyleVariants[25]); }
namespace { Gfx_RenderStyleController controller708B1C(g_RenderStyleVariants[26]); }
Gfx_RenderStyleController g_ShadowRenderStyle(g_RenderStyleVariants[27]);
namespace { Gfx_RenderStyleController controller708B34(g_RenderStyleVariants[28]); }
namespace { Gfx_RenderStyleController controller708A44(g_RenderStyleVariants[29]); }
namespace { Gfx_RenderStyleController controller708B7C(g_RenderStyleVariants[30]); }
namespace { Gfx_RenderStyleController controller708D44(g_RenderStyleVariants[31]); }
namespace { Gfx_RenderStyleController controller708ABC(g_RenderStyleVariants[32]); }
namespace { Gfx_RenderStyleController controller708C54(g_RenderStyleVariants[33]); }

Gfx_RenderStyleController* g_RenderStyles[34] =
{
	&controller708AD4, //0.
	&controller708AA4, //1.
	&controller708C84, //2.
	&controller708CFC, //3.
	&controller708BF4, //4.
	&controller708D2C, //5.
	&controller708C6C, //6.
	&controller708A74, //7.
	&controller708C3C, //8.
	&controller708BC4, //9.
	&controller708AEC, //10.
	&controller708D14, //11.
	&controller708BAC, //12.
	&controller708CCC, //13.
	&controller708C9C, //14.
	&controller708D5C, //15.
	&controller708C24, //16.
	&controller708C0C, //17.
	&controller708B4C, //18.
	&controller708CE4, //19.
	&controller708B04, //20.
	&controller708A5C, //21.
	&controller708B64, //22.
	&controller708B94, //23.
	&controller708CB4, //24.
	&controller708BDC, //25.
	&controller708B1C, //26.
	&g_ShadowRenderStyle, //27.
	&controller708B34, //28.
	&controller708A44, //29.
	&controller708B7C, //30.
	&controller708D44, //31.
	NULL, //32.
	NULL, //33.
};

namespace
{
	Gfx_RS_Base style70B790(3, 3, 0); //ctor0x0053B9F0,call0x0064211B.
	Gfx_RS_Base style70BB3C(3, 3, 1); //ctor0x0053B9F0,call0x0064213B.
	Gfx_RS_Base style70B850(4, 3, 0); //ctor0x0053B9F0,call0x0064215B.
	Gfx_RS_Base style70B780(5, 3, 0); //ctor0x0053B9F0,call0x0064217B.
	Gfx_RS_Base style70B860(4, 3, 1); //ctor0x0053B9F0,call0x0064219B.
	Gfx_RS_Base style70B5E4(5, 3, 1); //ctor0x0053B9F0,call0x006421BB.
	Gfx_RS_Base style70B8B4(4, 2, 1); //ctor0x0053B9F0,call0x006421DB.
	Gfx_RS_Base style70B50C(5, 2, 1); //ctor0x0053B9F0,call0x006421FB.
	Gfx_RS_Base style70B8A4(4, 4, 1); //ctor0x0053B9F0,call0x0064221B.
	Gfx_RS_Base style70BAA4(5, 4, 1); //ctor0x0053B9F0,call0x0064223B.
	Gfx_RS_B_LM style70B694(4, 3, 4, 3, 0); //ctor0x0053BEF0,call0x0064225F.
	Gfx_RS_B_LM style70B9C8(5, 3, 5, 3, 0); //ctor0x0053BEF0,call0x0064227F.
	Gfx_RS_B_LM style70B60C(4, 3, 4, 3, 1); //ctor0x0053BEF0,call0x0064229F.
	Gfx_RS_B_LM style70B8FC(5, 3, 5, 3, 1); //ctor0x0053BEF0,call0x006422BF.
	Gfx_RS_B_LM style70B8C4(4, 2, 4, 3, 1); //ctor0x0053BEF0,call0x006422DF.
	Gfx_RS_B_LM style70BA2C(5, 2, 5, 3, 1); //ctor0x0053BEF0,call0x006422FF.
	Gfx_RS_B_LM style70B39C(4, 4, 4, 4, 1); //ctor0x0053BEF0,call0x0064231F.
	Gfx_RS_B_LM style70B830(5, 4, 5, 4, 1); //ctor0x0053BEF0,call0x0064233F.
	Gfx_RS_B_LM style70B5C8(3, 3, 5, 3, 0); //ctor0x0053BEF0,call0x0064235F.
	Gfx_RS_B_EM style70B934(4, 3, 0); //ctor0x0053C600,call0x0064237B.
	Gfx_RS_B_EM style70B628(5, 3, 0); //ctor0x0053C600,call0x0064239B.
	Gfx_RS_B_EM style70B944(4, 3, 1); //ctor0x0053C600,call0x006423BB.
	Gfx_RS_B_EM style70B760(5, 3, 1); //ctor0x0053C600,call0x006423DB.
	Gfx_RS_B_EM style70B6F4(4, 2, 1); //ctor0x0053C600,call0x006423FB.
	Gfx_RS_B_EM style70B6B0(5, 2, 1); //ctor0x0053C600,call0x0064241B.
	Gfx_RS_B_EM style70B770(4, 4, 1); //ctor0x0053C600,call0x0064243B.
	Gfx_RS_B_EM style70B638(5, 4, 1); //ctor0x0053C600,call0x0064245B.
	Gfx_RS_Base_HeightFog style70B568(3, 3, 0); //ctor0x0053CC80,call0x0064247B.
	Gfx_RS_Base_HeightFog style70B418(3, 3, 1); //ctor0x0053CC80,call0x0064249B.
	Gfx_RS_Base_HeightFog style70B704(4, 3, 0); //ctor0x0053CC80,call0x006424BB.
	Gfx_RS_Base_HeightFog style70B730(5, 3, 0); //ctor0x0053CC80,call0x006424DB.
	Gfx_RS_Base_HeightFog style70B988(4, 3, 1); //ctor0x0053CC80,call0x006424FB.
	Gfx_RS_Base_HeightFog style70B3D4(5, 3, 1); //ctor0x0053CC80,call0x0064251B.
	Gfx_RS_Base_HeightFog style70B740(4, 2, 1); //ctor0x0053CC80,call0x0064253B.
	Gfx_RS_Base_HeightFog style70B38C(5, 2, 1); //ctor0x0053CC80,call0x0064255B.
	Gfx_RS_Base_HeightFog style70B334(4, 4, 1); //ctor0x0053CC80,call0x0064257B.
	Gfx_RS_Base_HeightFog style70B344(5, 4, 1); //ctor0x0053CC80,call0x0064259B.
	Gfx_RS_B_LM_HeightFog style70B3B8(4, 3, 4, 3, 0); //ctor0x0053D550,call0x006425BF.
	Gfx_RS_B_LM_HeightFog style70BA78(5, 3, 5, 3, 0); //ctor0x0053D550,call0x006425DF.
	Gfx_RS_B_LM_HeightFog style70B318(4, 2, 4, 3, 1); //ctor0x0053D550,call0x006425FF.
	Gfx_RS_B_LM_HeightFog style70B530(5, 2, 5, 3, 1); //ctor0x0053D550,call0x0064261F.
	Gfx_RS_B_LM_HeightFog style70B4C0(4, 3, 4, 3, 1); //ctor0x0053D550,call0x0064263F.
	Gfx_RS_B_LM_HeightFog style70BAC4(5, 3, 5, 3, 1); //ctor0x0053D550,call0x0064265F.
	Gfx_RS_B_LM_HeightFog style70B8E0(4, 4, 4, 4, 1); //ctor0x0053D550,call0x0064267F.
	Gfx_RS_B_LM_HeightFog style70B474(5, 4, 5, 4, 1); //ctor0x0053D550,call0x0064269F.
	Gfx_RS_B_LM_HeightFog style70B648(3, 3, 5, 3, 0); //ctor0x0053D550,call0x006426BF.
	Gfx_RS_Base_FOW style70B438(3, 3, 0); //ctor0x0053E0F0,call0x006426DB.
	Gfx_RS_Base_FOW style70BA00(3, 3, 1); //ctor0x0053E0F0,call0x006426FB.
	Gfx_RS_Base_FOW style70B51C(4, 3, 0); //ctor0x0053E0F0,call0x0064271B.
	Gfx_RS_Base_FOW style70BB28(5, 3, 0); //ctor0x0053E0F0,call0x0064273B.
	Gfx_RS_Base_FOW style70B490(4, 3, 1); //ctor0x0053E0F0,call0x0064275B.
	Gfx_RS_Base_FOW style70BA64(5, 3, 1); //ctor0x0053E0F0,call0x0064277B.
	Gfx_RS_Base_FOW style70B6D0(4, 2, 1); //ctor0x0053E0F0,call0x0064279B.
	Gfx_RS_Base_FOW style70B664(5, 2, 1); //ctor0x0053E0F0,call0x006427BB.
	Gfx_RS_Base_FOW style70B4DC(4, 4, 1); //ctor0x0053E0F0,call0x006427DB.
	Gfx_RS_Base_FOW style70B7E8(5, 4, 1); //ctor0x0053E0F0,call0x006427FB.
	Gfx_RS_B_LM_FOW style70B2E8(4, 3, 4, 3, 0); //ctor0x0053E9D0,call0x0064281F.
	Gfx_RS_B_LM_FOW style70B3E4(5, 3, 5, 3, 0); //ctor0x0053E9D0,call0x0064283F.
	Gfx_RS_B_LM_FOW style70B300(4, 2, 4, 3, 1); //ctor0x0053E9D0,call0x0064285F.
	Gfx_RS_B_LM_FOW style70B998(5, 2, 5, 3, 1); //ctor0x0053E9D0,call0x0064287F.
	Gfx_RS_B_LM_FOW style70B9B0(4, 3, 4, 3, 1); //ctor0x0053E9D0,call0x0064289F.
	Gfx_RS_B_LM_FOW style70B578(5, 3, 5, 3, 1); //ctor0x0053E9D0,call0x006428BF.
	Gfx_RS_B_LM_FOW style70B44C(4, 4, 4, 4, 1); //ctor0x0053E9D0,call0x006428DF.
	Gfx_RS_B_LM_FOW style70B7FC(5, 4, 5, 4, 1); //ctor0x0053E9D0,call0x006428FF.
	Gfx_RS_B_LM_FOW style70B970(3, 3, 5, 3, 0); //ctor0x0053E9D0,call0x0064291F.
	Gfx_RS_Base_HFog_FOW style70B428(3, 3, 0); //ctor0x0053F100,call0x0064293B.
	Gfx_RS_Base_HFog_FOW style70BAB4(3, 3, 1); //ctor0x0053F100,call0x0064295B.
	Gfx_RS_Base_HFog_FOW style70BB68(4, 3, 0); //ctor0x0053F100,call0x0064297B.
	Gfx_RS_Base_HFog_FOW style70B7D8(5, 3, 0); //ctor0x0053F100,call0x0064299B.
	Gfx_RS_Base_HFog_FOW style70BAE0(4, 3, 1); //ctor0x0053F100,call0x006429BB.
	Gfx_RS_Base_HFog_FOW style70B6C0(5, 3, 1); //ctor0x0053F100,call0x006429DB.
	Gfx_RS_Base_HFog_FOW style70B750(4, 2, 1); //ctor0x0053F100,call0x006429FB.
	Gfx_RS_Base_HFog_FOW style70B464(5, 2, 1); //ctor0x0053F100,call0x00642A1B.
	Gfx_RS_Base_HFog_FOW style70B6E4(4, 4, 1); //ctor0x0053F100,call0x00642A3B.
	Gfx_RS_Base_HFog_FOW style70BA94(5, 4, 1); //ctor0x0053F100,call0x00642A5B.
	Gfx_RS_B_LM_HFog_FOW style70B7A0(4, 3, 4, 3, 0); //ctor0x0053FBA0,call0x00642A7F.
	Gfx_RS_B_LM_HFog_FOW style70B354(5, 3, 5, 3, 0); //ctor0x0053FBA0,call0x00642A9F.
	Gfx_RS_B_LM_HFog_FOW style70B870(4, 2, 4, 3, 1); //ctor0x0053FBA0,call0x00642ABF.
	Gfx_RS_B_LM_HFog_FOW style70B954(5, 2, 5, 3, 1); //ctor0x0053FBA0,call0x00642ADF.
	Gfx_RS_B_LM_HFog_FOW style70B4F0(4, 3, 4, 3, 1); //ctor0x0053FBA0,call0x00642AFF.
	Gfx_RS_B_LM_HFog_FOW style70B9E4(5, 3, 5, 3, 1); //ctor0x0053FBA0,call0x00642B1F.
	Gfx_RS_B_LM_HFog_FOW style70B7BC(4, 4, 4, 4, 1); //ctor0x0053FBA0,call0x00642B3F.
	Gfx_RS_B_LM_HFog_FOW style70B5AC(5, 4, 5, 4, 1); //ctor0x0053FBA0,call0x00642B5F.
	Gfx_RS_B_LM_HFog_FOW style70BB0C(3, 3, 5, 3, 0); //ctor0x0053FBA0,call0x00642B7F.
	Gfx_RS_Expand style70B84C; //ctor0x00540940,call0x00642B95.
	Gfx_RS_SK_B_LM style70B814(5, 3, 7, 3, 0); //ctor0x00540BF0,call0x00642BBF.
	Gfx_RS_SK_B_LM style70B4A4(5, 3, 7, 3, 1); //ctor0x00540BF0,call0x00642BDF.
	Gfx_RS_SK_B_LM style70B54C(5, 2, 7, 3, 1); //ctor0x00540BF0,call0x00642BFF.
	Gfx_RS_SK_B_LM style70BA48(5, 4, 7, 4, 1); //ctor0x00540BF0,call0x00642C1F.
	Gfx_RS_SK_B_LM_HeightFog style70BB4C(5, 3, 7, 3, 0); //ctor0x00541340,call0x00642C3F.
	Gfx_RS_SK_B_LM_HeightFog style70B918(5, 3, 7, 3, 1); //ctor0x00541340,call0x00642C5F.
	Gfx_RS_SK_B_LM_HeightFog style70B3FC(5, 2, 7, 3, 1); //ctor0x00541340,call0x00642C7F.
	Gfx_RS_SK_B_LM_HeightFog style70BAF0(5, 4, 7, 4, 1); //ctor0x00541340,call0x00642C9F.
	Gfx_RS_SK_B_LM_FOW style70BA14(5, 3, 7, 3, 0); //ctor0x00541F90,call0x00642CBF.
	Gfx_RS_SK_B_LM_FOW style70B5F4(5, 3, 7, 3, 1); //ctor0x00541F90,call0x00642CDF.
	Gfx_RS_SK_B_LM_FOW style70B2D0(5, 2, 7, 3, 1); //ctor0x00541F90,call0x00642CFF.
	Gfx_RS_SK_B_LM_FOW style70B88C(5, 4, 7, 4, 1); //ctor0x00541F90,call0x00642D1F.
	Gfx_RS_SK_B_LM_HFog_FOW style70B678(5, 3, 7, 3, 0); //ctor0x00542740,call0x00642D3F.
	Gfx_RS_SK_B_LM_HFog_FOW style70B714(5, 3, 7, 3, 1); //ctor0x00542740,call0x00642D5F.
	Gfx_RS_SK_B_LM_HFog_FOW style70B590(5, 2, 7, 3, 1); //ctor0x00542740,call0x00642D7F.
	Gfx_RS_SK_B_LM_HFog_FOW style70B370(5, 4, 7, 4, 1); //ctor0x00542740,call0x00642D9F.
}

//Full original entry5061E0, not the requested inner address506210.
BYTE Gfx_InitializeRenderStyles(BYTE& singlePass)
{
	JE::LogDebugString("\n[Gfx: Initliazing Materials]\n\n");
	singlePass = 1;
	BYTE usesShader = 0;
	for (DWORD row = 0; row < 33; ++row)
		for (DWORD slot = 0; slot < 5; ++slot)
		{
			Gfx_RenderStyle** cell = &g_RenderStyleVariants[row][slot];
			if (*cell)
			{
				(*cell)->Initialize(); //HRESULT ignored; repeated cells are intentional.
				if ((*cell)->UsesVertexShader()) usesShader = 1; //Reload cell after callback.
				if ((*cell)->PassCount() > 1) singlePass = 0; //Reload again, unsigned comparison.
			}
		}
	JE::LogDebugString("[Gfx: Finished Initliazing Materials]\n\n");
	return usesShader;
}

//506260: void source-return hypothesis; original EAX is incidental.
void Gfx_ReleaseRenderStyles()
{
	for (DWORD row = 0; row < 33; ++row)
		for (DWORD slot = 0; slot < 5; ++slot)
			if (g_RenderStyleVariants[row][slot])
				g_RenderStyleVariants[row][slot]->Release();
}

//5367C0: real SDK effects, not the101 style virtual operations.
//Void source-return hypothesis; all HRESULTs ignored by the recovered callers.
void Gfx_NotifyRenderStylesLost()
{
	{
		ID3DXEffect* effect = g_RSBaseState.effect; //0070BBBC,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapState.normal; //0070BBDC,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapState.factored; //0070BBE0,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseEmissiveState.effect; //0070BC00,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseHeightFogState.effect; //0070BC20,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapHeightFogState.normal; //0070BC40,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapHeightFogState.factored; //0070BC44,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseFOWState.normal; //0070BC64,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseFOWState.factored; //0070BC68,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapFogOfWarState.effect; //0070BC88,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseHeightFogFOWState.effect; //0070BCA8,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapHeightFogFOWState.normal; //0070BCC8,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapHeightFogFOWState.factored; //0070BCCC,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSExpandState.effect; //0070BCEC,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapState.normal; //0070BD08,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapState.factored; //0070BD0C,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapHeightFogState.normal; //0070BD2C,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapHeightFogState.factored; //0070BD30,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapFOWState.effect; //0070BD50,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapHeightFogFOWState.normal; //0070BD70,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapHeightFogFOWState.factored; //0070BD74,read after prior callback.
		if (effect) effect->OnLostDevice();
	}
}

//536940: real SDK effects, not the101 style virtual operations.
//Void source-return hypothesis; all HRESULTs ignored by the recovered callers.
void Gfx_NotifyRenderStylesReset()
{
	{
		ID3DXEffect* effect = g_RSBaseState.effect; //0070BBBC,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapState.normal; //0070BBDC,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapState.factored; //0070BBE0,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseEmissiveState.effect; //0070BC00,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseHeightFogState.effect; //0070BC20,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapHeightFogState.normal; //0070BC40,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapHeightFogState.factored; //0070BC44,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseFOWState.normal; //0070BC64,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseFOWState.factored; //0070BC68,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapFogOfWarState.effect; //0070BC88,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseHeightFogFOWState.effect; //0070BCA8,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapHeightFogFOWState.normal; //0070BCC8,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSBaseLightmapHeightFogFOWState.factored; //0070BCCC,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSExpandState.effect; //0070BCEC,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapState.normal; //0070BD08,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapState.factored; //0070BD0C,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapHeightFogState.normal; //0070BD2C,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapHeightFogState.factored; //0070BD30,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapFOWState.effect; //0070BD50,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapHeightFogFOWState.normal; //0070BD70,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
	{
		ID3DXEffect* effect = g_RSSKBaseLightmapHeightFogFOWState.factored; //0070BD74,read after prior callback.
		if (effect) effect->OnResetDevice();
	}
}

