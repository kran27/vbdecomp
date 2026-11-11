#pragma once

#include <d3d8.h>
#include <string>

// Original frame array element: its string has normal compiler/library lifetime.
// Scalar fields have no default initializers; ResizeFrames writes only the
// evidenced subset. In particular firstMipSize/mipCount/padding stay unwritten.
struct Gfx_TextureFrame
{
	std::string name; // +00..1B under matching VC7.
	UINT width, height; // +1C/+20.
	DWORD bitsPerPixelWord; // +24; format helper writes a complete DWORD.
	DWORD totalSize, firstMipSize, mipCount; // +28/+2C/+30.
	D3DFORMAT format; // +34.
	D3DPOOL pool; // +38.
	DWORD value3C; // +3C; mip level saved by LockRect, consumed by UnlockRect.
	BYTE flag40; // +40; raw locked flag; padding+41..43 remains unwritten.
	IDirect3DTexture8* texture; // +44; owned COM reference.
};

class Gfx_Texture
{
public:
	explicit Gfx_Texture(int count); // 0x51F010.
	virtual ~Gfx_Texture(); // 0x51F280; real deleting destructor is table slot0.
	virtual HRESULT Bind(UINT stage, int frame); // +04, complete0x534C40..534C6D.
	void ResizeFrames(int count); // 0x535370.
	void UpdateMetadata(); // 0x534CE0.
	HRESULT LockRect(UINT level, D3DLOCKED_RECT* locked, const RECT* rectangle,
		DWORD flags, int frame); // 0x534F70; last argument is signed frame index.
	HRESULT UnlockRect(int frame); // 0x534BF0; consumes saved mip level.
	BYTE ClearFrame(int frame); // 0x535620; signed bounds, captured COM slot.
	void Clear(); //535170; clears pixel rows of every existing frame, retains ownership.
	HRESULT LoadFromBuffer(const BYTE* data, UINT size, D3DFORMAT format, D3DPOOL pool,
		UINT levels, DWORD filter, DWORD mipFilter, DWORD usage); // 0x535A80.
	HRESULT LoadFrameFromBuffer(const BYTE* data, UINT size, D3DFORMAT format,
		D3DPOOL pool, UINT levels, DWORD filter, DWORD mipFilter, DWORD usage,
		int frame); // 0x534EA0.
	HRESULT CreateTexture(UINT width, UINT height, UINT levels, D3DFORMAT format,
		D3DPOOL pool, DWORD usage, int frame); // 0x534DF0.
	Gfx_Texture* CloneDefaultToSystem(); // 0x5354B0; only default-pool frames copied.
	HRESULT CreateSystemTexture(std::string name, UINT width, UINT height, UINT levels,
		D3DFORMAT format, int frame); // 0x520100.
	BYTE CopyRect(UINT destinationMip, const RECT* destinationRect, Gfx_Texture* source,
		UINT sourceMip, const RECT* sourceRect, int sourceFrame, int destinationFrame); // 0x535010.
	HRESULT UpdateDefaultTexture(Gfx_Texture* source, int sourceFrame, int destinationFrame); // 0x535260.
	HRESULT LoadFromFile(std::string path, D3DFORMAT format, D3DPOOL pool, UINT levels,
		DWORD filter, DWORD mipFilter, DWORD usage); // 0x535CC0.
	HRESULT LoadFrameFromFile(std::string path, D3DFORMAT format, D3DPOOL pool,
		UINT levels, DWORD filter, DWORD mipFilter, DWORD usage, int frame); // 0x5356B0.
	HRESULT LoadAnimationBuffer(const BYTE* data, int size, D3DFORMAT format, D3DPOOL pool,
		UINT levels, DWORD filter, DWORD mipFilter, DWORD usage); // 0x535850.
	HRESULT LoadAnimationFile(std::string path, D3DFORMAT format, D3DPOOL pool,
		UINT levels, DWORD filter, DWORD mipFilter, DWORD usage); // 0x535B40.
	std::string GetDiagnosticName() const; //4AB1C0; actual thiscall result copy, no NULL guard.
	std::string GetFrameName(int frame) const; //520080; owning result, empty if out of range.
	// Native AddTexture inlines this guarded frame-name assignment.
	void SetFrameName(const std::string& value, int frame)
	{
		if (frame >= 0 && frame < frameCount) frames[frame].name = value;
	}

	std::string name; // +04; actual VC7 string.
	DWORD value20;
	Gfx_TextureFrame* frames; // +24; owned array with real compiler cookie/lifetime.
	int frameCount; // +28, signed.
	DWORD referenceCount; // +2C; initially1.
	DWORD value30;
};

//Compatibility facade for callers whose original source surface is still open.
std::string Gfx_GetTextureDiagnosticName(const Gfx_Texture* texture);
