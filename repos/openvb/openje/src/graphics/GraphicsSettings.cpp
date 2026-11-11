#include "Graphics.hpp"
#include "GraphicsServices.hpp"
#include <cstdlib>

namespace
{
	const JE::Configuration::Values* FindGraphicsSection(
		const JE::Configuration::Settings& settings)
	{
		JE::Configuration::Settings::const_iterator section = settings.find("Graphics");
		return section == settings.end() ? NULL : &section->second;
	}

	void ReadWord(const JE::Configuration::Values& values, const char* name, DWORD& output)
	{
		JE::Configuration::Values::const_iterator value = values.find(name);
		if (value != values.end()) output = static_cast<DWORD>(std::atol(value->second.c_str()));
	}

	void ReadByte(const JE::Configuration::Values& values, const char* name,
		BYTE& output, bool normalize)
	{
		JE::Configuration::Values::const_iterator value = values.find(name);
		if (value == values.end()) return;
		const long parsed = std::atol(value->second.c_str());
		output = normalize ? BYTE(parsed > 0) : static_cast<BYTE>(parsed);
	}

	void WriteValue(JE::Configuration::Values& values, const char* name, DWORD value)
	{
		char buffer[36];
		const char* text = ::_itoa(static_cast<int>(value), buffer, 10);
		values[name] = text;
	}
}

void Graphics::ReadSettings()
{
	//581C30 resets the display request and these flags, but neither shadowMode
	//nor bounds. The real owner fields remain the only configuration authority.
	adapter = 0;
	device = 0;
	displayMode = 0;
	fullscreen = 1;
	width = 800;
	height = 600;
	refresh = 0;
	color32 = 0;
	vsync = 1;
	statistics = 0;
	clearRed = 0;
	clearGreen = 0;
	clearBlue = 0;
	lighting = 1;
	linearFiltering = 1;
	startupMovies = 1;
	wireframe = 0;
	transparency = 1;
	const JE::Configuration::Values* values = FindGraphicsSection(JE::GraphicsServices::EngineSettings());
	if (values)
	{
		JE::Configuration::Values::const_iterator value = values->find("DebugOctree");
		if (value != values->end()) bounds = std::atol(value->second.c_str()) != 0;
	}
	values = FindGraphicsSection(JE::GraphicsServices::MainSettings());
	if (!values) return;
	ReadWord(*values, "adapter", adapter);
	ReadWord(*values, "device", device);
	ReadWord(*values, "displaymode", displayMode);
	ReadByte(*values, "fullscreen", fullscreen, true);
	ReadWord(*values, "width", width);
	ReadWord(*values, "height", height);
	ReadWord(*values, "refresh", refresh);
	ReadByte(*values, "mode32bpp", color32, true);
	ReadByte(*values, "stats", statistics, true);
	ReadByte(*values, "clear red", clearRed, false);
	ReadByte(*values, "clear green", clearGreen, false);
	ReadByte(*values, "clear blue", clearBlue, false);
	JE::Configuration::Values::const_iterator shadow = values->find("shadow type");
	if (shadow != values->end())
	{
		const DWORD parsed = static_cast<DWORD>(std::atol(shadow->second.c_str()));
		shadowMode = parsed < 3 ? parsed : 1;
	}
	ReadByte(*values, "enable lighting", lighting, true);
	ReadByte(*values, "enable bilinear filtering", linearFiltering, true);
	ReadByte(*values, "enable startup movies", startupMovies, true);
	ReadByte(*values, "vsync", vsync, true);
	ReadByte(*values, "enable wireframe", wireframe, true);
	ReadByte(*values, "enable transparency", transparency, true);
}

void Graphics::WriteSettings()
{
	//582BB0 mutates the original main settings object; disk save is its caller's policy.
	JE::Configuration::Values& values = JE::GraphicsServices::MainSettings()["Graphics"];
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
