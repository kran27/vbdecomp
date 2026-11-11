#include "Texture.hpp"

std::string Gfx_Texture::GetDiagnosticName() const //4AB1C0.
{
	return name;
}

std::string Gfx_GetTextureDiagnosticName(const Gfx_Texture* texture)
{
	return texture->GetDiagnosticName();
}

std::string Gfx_Texture::GetFrameName(int frame) const //520080.
{
	if (frame >= 0 && frame < frameCount) return frames[frame].name;
	return "";
}
