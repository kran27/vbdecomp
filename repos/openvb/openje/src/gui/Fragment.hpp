#pragma once

#include "Base.hpp"
#include "graphics/MeshData.hpp"
#include "graphics/UIVertex.hpp"

class Gfx_Viewport;

namespace GUI
{
	// Original table0x66EA1C: fifteen Base slots followed by Draw and UpdateUV.
	// Lifetime uses the actual recovered members. Required render/resource bodies
	// remain undefined, so this class is not yet linked into the legacy game.
	class Fragment : public Base
	{
	public:
		Fragment(); // 0x4B24A0; complete original includes exception chunks.
		virtual ~Fragment(); // 0x4B2680.
		virtual void Resize(LONG width, LONG height); // +10, 0x4B1450.
		virtual bool Contains(LONG x, LONG y); // +18, 0x4B1650.
		virtual BYTE ReadResource(JE::Resources::ResourceFileHandle* file, int version); // +1C, 0x4B20A0.
		virtual BYTE ReadFile(FILE* file, int version); // +20, 0x4B1E50.
		virtual BYTE WriteFile(FILE* file); // +24, 0x4B2300.
		virtual void Refresh(); // +28, 0x4B1600, original tail-calls Base.
		virtual float SetDepth(float depth, float increment); // +30, 0x4B1610.
		virtual float GetDepth(); // +34, 0x4B1640.
		virtual void UpdateGeometry(); // +38, 0x4B1830.
		virtual void Draw(Gfx_Viewport* target, BYTE flags); // +3C, 0x4B1720.
		virtual void UpdateUV(); // +40, 0x4B1B00.

		void SetTextureHandle(DWORD handle); // 0x4B1470; first handle, dirties UV only.
		DWORD GetTextureHandle(); // 0x4B1490.
		DWORD GetTileMode();
		void SetTileMode(DWORD mode); // 0x4B14A0.
		void SetColor(DWORD color); // 0x4B14C0; regroup only crossing opaque alpha.
		DWORD GetColor(); // 0x4B1530.
		void SetFrameRate(DWORD rate); // 0x4B1540.
		DWORD GetFrameRate();
		DWORD GetFrameCount();
		void SetFrameSize(DWORD width, DWORD height);
		void GetFrameSize(DWORD& width, DWORD& height); // 0x4B1550.
		void SetFrameCount(DWORD count); // 0x4B1570.
		void SetMaterialModes(DWORD value); // 0x4B1580; exact mode meaning unresolved.
		void SetSourceRect(float x, float y, float width, float height); // 0x4B15A0.
		void GetSourceRect(float& x, float& y, float& width, float& height); // 0x4B15D0.

		BYTE uvDirty; // +18; padding+19..1B is unwritten.
		DWORD tileMode; // +1C.
		DWORD frameWidth, frameHeight, frameRate, currentFrame; // +20..2C.
		DWORD frameCount, lastFrameTime; // +30/+34; constructor leaves both unwritten.
		float sourceX, sourceY, sourceWidth, sourceHeight; // +38..44.
		Gfx_MeshData mesh; // +48..B3; actual owned member.
		Gfx_Primitive* primitive; // +B4; borrowed from mesh.primitives.
		JE::Gfx::UIVertex vertices[4]; // +B8..127; X/Y remain unwritten initially.
		Gfx_Material* material; // +128; borrowed from mesh.materialSet.
		Gfx_MeshInstance instance; // +12C..153; actual owned member.
		DWORD color; // +154.
	};
}
