#pragma once

#include "RenderQueue.hpp"
#include <list>

namespace JE
{
namespace Gfx
{
	class Viewport
	{
	public:
		Viewport(D3DContext& context, GeometryStreams& streams, BaseRenderStyles& styles);
		void Render(BYTE clear); //0x4BFF20.
		D3DVIEWPORT8 rectangle;
		D3DMATRIX projection, view, cameraTransform;
		float aspect, fov;
		float nearPlane, farPlane;
		BYTE projectionDirty, viewDirty;
		DWORD clearMask, clearColor, stencil;
		float depth;
		BYTE suppressColorOnce, fog;
		DWORD fogStart, fogEnd, fogDensity;
		RenderQueue queue;
	};

	// Bounded native lower renderer. This does not stand in for Graphics/Gfx_Engine.
	class FrameRenderer
	{
	public:
		FrameRenderer(D3DContext& context, GeometryStreams& streams, BaseRenderStyles& styles);
		void SetDimensions(UINT width, UINT height, BYTE hasStencil);
		HRESULT SetTransparency(BYTE enabled); //0x5BDE20; Graphics::Configure setting.
		void SubmitViewport(Viewport& viewport); //0x5C1B00.
		bool RenderFrame(BYTE clearQueues, BYTE present); //0x5BFCB0.
		void Update(); // Normal0x5C1940 path; lost-device recovery remains explicit.
		void UpdateStatistics(); //0x502BE0.
		D3DContext& context;
		Viewport defaultViewport, postViewport;
		Viewport* currentViewport;
		std::list<Viewport*> additionalViewports;
		DWORD lost;
		DWORD& begun;
		DWORD& ended;
		BYTE statistics, debugFrames;
		BYTE transparency;
		BYTE fogOfWar, projectedShadows, automap;
		float previousStatisticsTime, currentStatisticsTime;
	};

	namespace FrameServices
	{
		HRESULT BeginScene(D3DContext& context);
		HRESULT EndScene(D3DContext& context);
		HRESULT Present(D3DContext& context, HWND window);
		HRESULT CooperativeLevel(D3DContext& context);
		HRESULT SetViewport(D3DContext& context, const D3DVIEWPORT8& rectangle);
		HRESULT Clear(D3DContext& context, DWORD mask, DWORD color, float depth, DWORD stencil);
		void PrepareCamera(Viewport& viewport);
		void RenderQueueBody(RenderQueue& queue, BYTE clear);
		void ClearQueueBody(RenderQueue& queue);
		void RenderViewport(Viewport& viewport, BYTE clear);
		void PerformanceCounter(LARGE_INTEGER& counter);
		void Sleep(DWORD milliseconds);
		void Debug(const char* message);
	}
}
}
