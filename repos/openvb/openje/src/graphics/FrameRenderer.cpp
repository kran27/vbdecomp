#include "FrameRenderer.hpp"
#include "diagnostics/DebugOutput.hpp"

#include <d3dx8math.h>
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace JE
{
namespace Gfx
{
	Viewport::Viewport(D3DContext& context, GeometryStreams& streams, BaseRenderStyles& styles)
		: aspect(1.3333333730697632f), fov(15.0f * 0.017453292f), nearPlane(10.0f),
		farPlane(20000.0f), projectionDirty(1), viewDirty(1), clearMask(3),
		clearColor(0xFF000100u), stencil(0), depth(1.0f), suppressColorOnce(0), fog(0),
		fogStart(0), fogEnd(0x3F800000u), fogDensity(0x3F800000u), queue(context, streams, styles)
	{
		const D3DVIEWPORT8 initial = {0, 0, 0, 0, 0.0f, 1.0f};
		rectangle = initial;
		queue.owner = this;
		cameraTransform = context.identity;
		projection = context.identity;
		view = context.identity;
	}

	void Viewport::Render(BYTE clear)
	{
		FrameServices::PrepareCamera(*this);
		D3DContext& context = queue.context;
		context.SetRenderState(D3DRS_FOGENABLE, fog);
		if (fog)
		{
			context.SetRenderState(D3DRS_FOGSTART, fogStart);
			context.SetRenderState(D3DRS_FOGEND, fogEnd);
			context.SetRenderState(D3DRS_FOGDENSITY, fogDensity);
		}
		DWORD mask = clearMask;
		if (mask)
		{
			if (suppressColorOnce)
			{
				mask &= ~DWORD(D3DCLEAR_TARGET);
				suppressColorOnce = 0;
			}
			FrameServices::Clear(context, mask, clearColor, depth, stencil);
		}
		FrameServices::RenderQueueBody(queue, clear);
	}

	FrameRenderer::FrameRenderer(D3DContext& context, GeometryStreams& streams,
		BaseRenderStyles& styles) : context(context), defaultViewport(context, streams, styles),
		postViewport(context, streams, styles), currentViewport(NULL), lost(0),
		begun(context.statistics[6]), ended(context.statistics[7]), statistics(0),
		debugFrames(0), transparency(0), fogOfWar(0), projectedShadows(0), automap(0),
		previousStatisticsTime(0), currentStatisticsTime(0)
	{
	}

	void FrameRenderer::SetDimensions(UINT width, UINT height, BYTE hasStencil)
	{
		defaultViewport.queue.streams.activeLights =
			(std::min)(DWORD(8), context.currentDevice->caps.MaxActiveLights);
		const D3DVIEWPORT8 rectangle = {0, 0, width, height, 0.0f, 1.0f};
		defaultViewport.rectangle = rectangle;
		postViewport.rectangle = defaultViewport.rectangle;
		defaultViewport.clearColor = 0xFF002080u;
		defaultViewport.clearMask = hasStencil ? 7 : 3;
		postViewport.clearColor = 0;
		postViewport.clearMask = 2;
		currentViewport = &defaultViewport;
	}

	void FrameRenderer::SubmitViewport(Viewport& viewport)
	{
		additionalViewports.push_back(&viewport);
	}

	HRESULT FrameRenderer::SetTransparency(BYTE enabled)
	{
		context.SetRenderState(D3DRS_ALPHATESTENABLE, enabled ? 1 : 0);
		const HRESULT result = context.SetRenderState(D3DRS_ALPHAFUNC, enabled ? 5 : 8);
		transparency = enabled;
		return result;
	}

	bool FrameRenderer::RenderFrame(BYTE clearQueues, BYTE present)
	{
		++begun;
		if (FAILED(FrameServices::BeginScene(context)))
			return false;
		if (debugFrames)
			FrameServices::Debug("\n---BeginScene\n");
		currentViewport = &defaultViewport;
		FrameServices::SetViewport(context, currentViewport->rectangle);
		FrameServices::RenderViewport(defaultViewport, clearQueues);
		for (std::list<Viewport*>::iterator current = additionalViewports.begin(); current != additionalViewports.end(); ++current)
		{
			currentViewport = *current;
			FrameServices::SetViewport(context, currentViewport->rectangle);
			FrameServices::RenderViewport(*currentViewport, clearQueues);
		}
		currentViewport = &postViewport;
		FrameServices::SetViewport(context, currentViewport->rectangle);
		FrameServices::RenderViewport(postViewport, clearQueues);
		if (statistics)
		{
			context.SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
			FrameServices::SetViewport(context, defaultViewport.rectangle);
			throw std::logic_error("Unrecovered RenderStatistics child of0x5BFCB0");
		}
		++ended;
		FrameServices::EndScene(context);
		if (debugFrames)
			FrameServices::Debug("---EndScene\n");
		if (present && FrameServices::Present(context, context.window) == D3DERR_DEVICELOST)
			lost = 1;
		UpdateStatistics();
		return true;
	}

	void FrameRenderer::Update()
	{
		if (!context.device)
			return;
		if (FrameServices::CooperativeLevel(context) == D3DERR_DEVICELOST)
			lost = 1;
		if (lost == 1)
		{
			const HRESULT result = FrameServices::CooperativeLevel(context);
			if (result == D3DERR_DEVICENOTRESET)
			{
				JE::LogDebugString("Release Video Resources\n");
				throw std::logic_error("Unrecovered graphics resource release0x5BEDF0");
			}
			if (result)
			{
				FrameServices::ClearQueueBody(defaultViewport.queue);
				FrameServices::ClearQueueBody(postViewport.queue);
				for (std::list<Viewport*>::iterator entry = additionalViewports.begin(); entry != additionalViewports.end(); ++entry)
					FrameServices::ClearQueueBody((*entry)->queue);
				FrameServices::Sleep(1000);
			}
			else
			{
				JE::LogDebugString("TestCooperativeLevel() failed: %x\n", 0);
				lost = 0;
			}
			return;
		}
		if (fogOfWar)
			throw std::logic_error("Unrecovered ProcessFogOfWar0x5BE380");
		if (projectedShadows)
			throw std::logic_error("Unrecovered ProjectShadows0x5BEF80");
		if (automap)
			throw std::logic_error("Unrecovered RenderAutomap0x5C1000");
		RenderFrame(1, 1);
		additionalViewports.clear();
	}

	void FrameRenderer::UpdateStatistics()
	{
		++context.statistics[2];
		++context.statistics[1];
		LARGE_INTEGER counter;
		FrameServices::PerformanceCounter(counter);
		currentStatisticsTime = static_cast<float>(
			static_cast<double>(context.inverseFrequency) * static_cast<double>(counter.QuadPart));
		const double elapsed = static_cast<double>(context.inverseFrequency) *
			static_cast<double>(counter.QuadPart) - previousStatisticsTime;
		if (elapsed > 0.5)
		{
			const float framesPerSecond = static_cast<float>(context.statistics[1] / elapsed);
			context.statistics[1] = 0;
			std::memcpy(&context.statistics[0], &framesPerSecond, sizeof(framesPerSecond));
			previousStatisticsTime = currentStatisticsTime;
		}
		for (unsigned i = 3; i <= 13; ++i)
			context.statistics[i] = 0;
	}

	namespace FrameServices
	{
		HRESULT BeginScene(D3DContext& context) { return context.device->BeginScene(); }
		HRESULT EndScene(D3DContext& context) { return context.device->EndScene(); }
		HRESULT Present(D3DContext& context, HWND window)
		{
			return context.device->Present(NULL, NULL, window, NULL);
		}
		HRESULT CooperativeLevel(D3DContext& context) { return context.device->TestCooperativeLevel(); }
		HRESULT SetViewport(D3DContext& context, const D3DVIEWPORT8& rectangle)
		{
			return context.device->SetViewport(&rectangle);
		}
		HRESULT Clear(D3DContext& context, DWORD mask, DWORD color, float depth, DWORD stencil)
		{
			return context.device->Clear(0, NULL, mask, color, depth, stencil);
		}
		void PrepareCamera(Viewport& viewport)
		{
			if (viewport.projectionDirty)
			{
				D3DXMATRIX matrix;
				D3DXMatrixPerspectiveFovLH(&matrix, viewport.fov, viewport.aspect,
					viewport.nearPlane, viewport.farPlane);
				viewport.projection = matrix;
				viewport.projectionDirty = 0;
			}
			D3DContext& context = viewport.queue.context;
			context.SetTransform(D3DTS_PROJECTION, &viewport.projection);
			if (viewport.viewDirty)
			{
				const D3DMATRIX& source = viewport.cameraTransform;
				D3DMATRIX& view = viewport.view;
				for (unsigned row = 0; row < 3; ++row)
					for (unsigned column = 0; column < 3; ++column)
						view.m[row][column] = source.m[column][row];
				for (unsigned column = 0; column < 3; ++column)
					view.m[3][column] = -(source.m[3][2] * source.m[column][2] +
						source.m[3][1] * source.m[column][1] + source.m[3][0] * source.m[column][0]);
				viewport.viewDirty = 0;
			}
			context.SetTransform(D3DTS_VIEW, &viewport.view);
		}
		void RenderQueueBody(RenderQueue& queue, BYTE clear) { queue.Render(clear); }
		void ClearQueueBody(RenderQueue& queue) { queue.Clear(); }
		void RenderViewport(Viewport& viewport, BYTE clear) { viewport.Render(clear); }
		void PerformanceCounter(LARGE_INTEGER& counter) { QueryPerformanceCounter(&counter); }
		void Sleep(DWORD milliseconds) { ::Sleep(milliseconds); }
		void Debug(const char* message) { OutputDebugStringA(message); }
	}
}
}
