#pragma once

#include "D3DContext.hpp"
#include "UIVertex.hpp"
#include <list>
#include <vector>

namespace JE
{
namespace Gfx
{
	struct UIMaterial
	{
		UIMaterial(); // Known defaults from0x4B60B0.
		DWORD style, renderPass;
		IDirect3DBaseTexture8* texture; // Borrowed from the texture owner.
		D3DMATERIAL8 values;
		BYTE textureFactorColor, textureFactorAlpha;
		BYTE vertexColor, cull, depthTest, lighting, fog;
	};

	struct UIMesh
	{
		UIMesh() : primitive(D3DPT_TRIANGLESTRIP), primitiveCount(2) {}
		std::vector<UIVertex> vertices;
		std::vector<WORD> indices;
		D3DPRIMITIVETYPE primitive;
		UINT primitiveCount;
		UIMaterial material;
	};

	class BaseRenderStyles;
	class Viewport;

	// The recovered dynamic UI streams. The device and caches remain D3DContext's.
	class GeometryStreams
	{
	public:
		explicit GeometryStreams(D3DContext& context) : context(context), vertices(NULL),
			indices(NULL), vertexCapacity(0), vertexPosition(0), indexCapacity(0),
			indexBytes(0), indexPosition(0), indexStride(0), activeLights(0) {}
		~GeometryStreams();
		void Initialize(BYTE softwareVertexProcessing); //0x5067C0.
		void Release();
		HRESULT LockVertices(UINT count, UINT stride, BYTE** output, UINT* base,
			BYTE discard = 0); //0x505400.
		HRESULT LockIndices(UINT count, BYTE** output, UINT* base, BYTE discard = 0);
		D3DContext& context;
		IDirect3DVertexBuffer8* vertices;
		IDirect3DIndexBuffer8* indices;
		UINT vertexCapacity, vertexPosition;
		UINT indexCapacity, indexBytes, indexPosition, indexStride;
		// Shared backend light limit from0x5C0B40; native UI composition state.
		UINT activeLights;
	private:
		GeometryStreams(const GeometryStreams&);
		GeometryStreams& operator=(const GeometryStreams&);
	};

	class RenderQueue
	{
	public:
		RenderQueue(D3DContext& context, GeometryStreams& streams, BaseRenderStyles& styles)
			: context(context), streams(streams), styles(styles), owner(NULL),
			stencilShadows(0), floor(0), extraPass(0), lighting(false) {}
		void Publish(UIMesh& mesh); // Dynamic UI branch of0x4948A0.
		void Render(BYTE clear); //0x494CA0; populated UI, other producers unrecovered.
		void Clear();
		D3DContext& context;
		GeometryStreams& streams;
		BaseRenderStyles& styles;
		struct QueuedMesh
		{
			QueuedMesh() : vertexBase(0), indexBase(0) {}
			UIMesh* mesh;
			UINT vertexBase, indexBase;
		};
		std::list<QueuedMesh> meshes; // Borrows native mesh objects until Clear.
		Viewport* owner;
		BYTE stencilShadows, floor, extraPass;
		bool lighting;
	private:
		void DrawPass(UINT pass, BYTE shadowOnly);
		void RenderEmptySkins(UINT pass);
	};

	struct UIBatchClass
	{
		enum Type
		{
		OpaqueQuads = 0,
		Direct = 1,
		Overlay = 2,
		Text = 3
		};
	};

	class UIBatcher
	{
	public:
		explicit UIBatcher(Viewport& viewport) : viewport(viewport) {}
		void BeginFrame(); //0x4B6450 plus native storage/real-library pool boundary.
		void Submit(UIMesh& mesh, Viewport* target = NULL,
			UIBatchClass::Type kind = UIBatchClass::OpaqueQuads); //0x4B6750.
		void Flush(); //0x4B7140 ->0x4B6F90/0x4B7050.
		Viewport& viewport;
		std::list<UIMesh*> pending[4];
		std::list<UIMesh> generated; // Owns generated batches through the next BeginFrame.
	private:
		void FlushQuads();
		void FlushText();
	};

	DWORD MaterialFactor(const UIMaterial& material); //0x4B6030 packed diffuse.
	void SetMaterialFactor(UIMaterial& material, DWORD color, BYTE useColor = 1,
		BYTE useAlpha = 1); //0x4B5EB0, through0x4AB600 for one UI material.

	namespace RenderServices
	{
		HRESULT CreateVertices(D3DContext&, UINT, DWORD, IDirect3DVertexBuffer8**);
		HRESULT CreateIndices(D3DContext&, UINT, DWORD, D3DFORMAT, IDirect3DIndexBuffer8**);
		ULONG Release(IDirect3DVertexBuffer8*);
		ULONG Release(IDirect3DIndexBuffer8*);
		HRESULT Lock(IDirect3DVertexBuffer8*, UINT, UINT, BYTE**, DWORD);
		HRESULT Lock(IDirect3DIndexBuffer8*, UINT, UINT, BYTE**, DWORD);
		HRESULT Unlock(IDirect3DVertexBuffer8*);
		HRESULT Unlock(IDirect3DIndexBuffer8*);
		HRESULT Stream(D3DContext&, IDirect3DVertexBuffer8*, UINT);
		HRESULT Indices(D3DContext&, IDirect3DIndexBuffer8*, UINT);
		HRESULT Shader(D3DContext&, DWORD);
		HRESULT EnableLight(D3DContext&, UINT, BOOL);
		HRESULT Draw(D3DContext&, D3DPRIMITIVETYPE, UINT, UINT);
		HRESULT DrawIndexed(D3DContext&, D3DPRIMITIVETYPE, UINT, UINT, UINT, UINT);
	}
}
}
