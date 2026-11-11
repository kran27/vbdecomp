#include "GraphicsConfiguration.hpp"
#include "SystemInterface.hpp"
#include "D3DState.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstdlib>
#include <stdexcept>

namespace JE
{
	namespace
	{
		const Configuration::Values* Section(const Configuration::Settings& settings)
		{
			Configuration::Settings::const_iterator i = settings.find("Graphics");
			return i == settings.end() ? NULL : &i->second;
		}
		void ReadWord(const Configuration::Values& values, const char* key, DWORD& value)
		{
			Configuration::Values::const_iterator i = values.find(key);
			if (i != values.end()) value = static_cast<DWORD>(std::atol(i->second.c_str()));
		}
		void ReadByte(const Configuration::Values& values, const char* key, BYTE& value, BYTE flag)
		{
			Configuration::Values::const_iterator i = values.find(key);
			if (i != values.end())
			{
				const long parsed = std::atol(i->second.c_str());
				value = flag ? BYTE(parsed > 0) : static_cast<BYTE>(parsed);
			}
		}
		void WriteValue(Configuration::Values& values, const char* key, DWORD value)
		{
			char text[36];
			values[key] = ::_itoa(static_cast<int>(value), text, 10);
		}
	}

	GraphicsConfiguration::GraphicsConfiguration()
		: initialized(0), adapter(0), device(0), displayMode(0), vsync(1), statistics(0),
		clearRed(0), clearGreen(64), clearBlue(64), shadowMode(0), lighting(1),
		linearFiltering(1), startupMovies(1), wireframe(0), transparency(1), bounds(0)
	{
		//580E20 leaves fullscreen/width/height/refresh/color32 unwritten.
	}

	void GraphicsConfiguration::Read(const Configuration::Settings& engine,
		const Configuration::Settings& main)
	{
		adapter = device = displayMode = 0;
		fullscreen = 1;
		width = 800; height = 600; refresh = 0;
		color32 = 0; vsync = 1; statistics = 0;
		clearRed = clearGreen = clearBlue = 0;
		lighting = linearFiltering = startupMovies = 1;
		wireframe = 0; transparency = 1;
		//581C30 does not reset shadowMode or bounds before the lookups.
		const Configuration::Values* values = Section(engine);
		if (values)
		{
			Configuration::Values::const_iterator i = values->find("DebugOctree");
			if (i != values->end()) bounds = std::atol(i->second.c_str()) != 0;
		}
		values = Section(main);
		if (!values) return;
		ReadWord(*values, "adapter", adapter);
		ReadWord(*values, "device", device);
		ReadWord(*values, "displaymode", displayMode);
		ReadByte(*values, "fullscreen", fullscreen, 1);
		ReadWord(*values, "width", width);
		ReadWord(*values, "height", height);
		ReadWord(*values, "refresh", refresh);
		ReadByte(*values, "mode32bpp", color32, 1);
		ReadByte(*values, "stats", statistics, 1);
		ReadByte(*values, "clear red", clearRed, 0);
		ReadByte(*values, "clear green", clearGreen, 0);
		ReadByte(*values, "clear blue", clearBlue, 0);
		Configuration::Values::const_iterator shadow = values->find("shadow type");
		if (shadow != values->end())
		{
			const DWORD parsed = static_cast<DWORD>(std::atol(shadow->second.c_str()));
			shadowMode = parsed < 3 ? parsed : 1; //Unsigned582554; negatives also select1.
		}
		ReadByte(*values, "enable lighting", lighting, 1);
		ReadByte(*values, "enable bilinear filtering", linearFiltering, 1);
		ReadByte(*values, "enable startup movies", startupMovies, 1);
		ReadByte(*values, "vsync", vsync, 1);
		ReadByte(*values, "enable wireframe", wireframe, 1);
		ReadByte(*values, "enable transparency", transparency, 1);
	}

	void GraphicsConfiguration::Write(Configuration::Settings& main) const
	{
		Configuration::Values& values = main["Graphics"];
		WriteValue(values, "adapter", adapter);
		WriteValue(values, "device", device);
		WriteValue(values, "displaymode", displayMode);
		WriteValue(values, "fullscreen", fullscreen);
		WriteValue(values, "width", width);
		WriteValue(values, "height", height);
		WriteValue(values, "refresh", refresh);
		WriteValue(values, "mode32bpp", color32);
		WriteValue(values, "stats", statistics);
		WriteValue(values, "clear red", clearRed);
		WriteValue(values, "clear green", clearGreen);
		WriteValue(values, "clear blue", clearBlue);
		WriteValue(values, "shadow type", shadowMode);
		WriteValue(values, "enable lighting", lighting);
		WriteValue(values, "enable bilinear filtering", linearFiltering);
		WriteValue(values, "enable startup movies", startupMovies);
		WriteValue(values, "vsync", vsync);
		WriteValue(values, "enable wireframe", wireframe);
		WriteValue(values, "enable transparency", transparency);
	}

	GraphicsDisplayRequest SelectGraphicsDisplayMode(GraphicsConfiguration& state,
		Gfx_System_Interface*& backend)
	{
		if (state.initialized)
			throw std::logic_error("Graphics581820: repeat configuration requires actual frontend cleanup581490");
		GraphicsDisplayRequest request;
		request.fullscreen = state.fullscreen;
		request.width = state.width;
		request.height = state.height;
		request.refresh = state.refresh;
		request.color32 = state.color32;
		state.initialized = 1; //Original581834, before all adapter/device callbacks.
		const DWORD adapters = backend->AdapterCount();
		if (state.adapter >= adapters) state.adapter = 0; //Live field load follows callback.
		const DWORD devices = backend->DeviceCount(state.adapter);
		if (state.device >= devices) state.device = 0;
		backend->ModeCount(state.adapter, state.device); //58188E, unused result retained.
		backend->Modes(state.adapter, state.device); //5818A1, likewise.
		if (!backend->FindMode(state.displayMode, state.adapter, state.device,
			request.width, request.height, request.color32, request.refresh))
			FatalError("Cannot find requested mode.\n");
		request.modes = backend->Modes(state.adapter, state.device);
		Gfx_D3DMode& selected = request.modes[state.displayMode];
		if (!selected.stencil)
			LogDebugString("@@@ Selected Displaymode does NOT support stencil buffers: %d x %d x %dbpp DisplayFormat = %d DepthStencil = %d\n",
				selected.display.Width, selected.display.Height,
				selected.backBufferFormat == D3DFMT_X8R8G8B8 || selected.backBufferFormat == D3DFMT_A8R8G8B8 ? 32 : 16,
				selected.backBufferFormat, selected.depthStencilFormat);
		return request;
	}

	void PublishGraphicsDisplayMode(GraphicsConfiguration& state,
		const GraphicsDisplayRequest& request)
	{
		state.fullscreen = request.fullscreen;
		state.width = request.width;
		state.height = request.height;
		state.refresh = request.refresh;
		const D3DFORMAT format = request.modes[state.displayMode].backBufferFormat;
		state.color32 = format == D3DFMT_X8R8G8B8 || format == D3DFMT_A8R8G8B8;
	}
}
