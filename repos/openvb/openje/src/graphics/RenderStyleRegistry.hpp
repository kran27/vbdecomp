#pragma once

#include "RenderStyle.hpp"

extern Gfx_RenderStyle* g_RenderStyleVariants[34][5]; //6FBD20;33 rows traversed.

BYTE Gfx_InitializeRenderStyles(BYTE& singlePass); //5061E0; HRESULTs ignored.
void Gfx_ReleaseRenderStyles(); //506260; includes repeated cells.
void Gfx_NotifyRenderStylesLost(); //5367C0;21 shared SDK effects, fixed order.
void Gfx_NotifyRenderStylesReset(); //536940;21 shared SDK effects, fixed order.
