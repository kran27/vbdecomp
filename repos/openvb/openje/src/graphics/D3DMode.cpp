#include "D3DState.hpp"

Gfx_D3DMode::Gfx_D3DMode()
{
	backBufferFormat = D3DFMT_UNKNOWN;
	depthStencilFormat = D3DFMT_UNKNOWN;
	behavior = 0;
	stencil = 0;
	display.Width = 0;
	display.Height = 0;
	display.Format = D3DFMT_UNKNOWN;
	display.RefreshRate = 0;
}
