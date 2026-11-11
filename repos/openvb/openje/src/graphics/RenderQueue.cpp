#include "RenderQueue.hpp"
#include "FrameRenderer.hpp"
#include "BaseRenderStyle.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace JE
{
namespace Gfx
{
	UIMaterial::UIMaterial() : style(20), renderPass(1), texture(NULL),
		textureFactorColor(0), textureFactorAlpha(0), vertexColor(1), cull(1),
		depthTest(1), lighting(1), fog(0)
	{
		const D3DCOLORVALUE white = {1, 1, 1, 1};
		const D3DCOLORVALUE black = {0, 0, 0, 1};
		values.Diffuse = white;
		values.Ambient = white;
		values.Specular = white;
		values.Emissive = black;
		values.Power = 0;
	}

	DWORD MaterialFactor(const UIMaterial& material)
	{
		const D3DCOLORVALUE& color = material.values.Diffuse;
		return (static_cast<DWORD>(color.b * 255.0) & 255) |
			((static_cast<DWORD>(color.g * 255.0) & 255) << 8) |
			((static_cast<DWORD>(color.r * 255.0) & 255) << 16) |
			(static_cast<DWORD>(color.a * 255.0) << 24);
	}

	void SetMaterialFactor(UIMaterial& material, DWORD color, BYTE useColor, BYTE useAlpha)
	{
		const float scale = 0.0039215689f;
		if (useColor)
		{
			material.values.Diffuse.r = ((color >> 16) & 255) * scale;
			material.values.Diffuse.g = ((color >> 8) & 255) * scale;
			material.values.Diffuse.b = (color & 255) * scale;
		}
		if (useAlpha)
			material.values.Diffuse.a = (color >> 24) * scale;
		material.values.Ambient = material.values.Diffuse;
		material.textureFactorColor = useColor;
		material.textureFactorAlpha = useAlpha;
	}

	GeometryStreams::~GeometryStreams()
	{
		Release();
	}

	void GeometryStreams::Initialize(BYTE softwareVertexProcessing)
	{
		const DWORD usage = 0x208u | (softwareVertexProcessing ? 0 : 0x20u);
		if (!vertices)
		{
			vertexCapacity = 1835008;
			RenderServices::CreateVertices(context, vertexCapacity, usage, &vertices);
		}
		if (!indices)
		{
			indexCapacity = 393216;
			indexStride = context.field0xA4;
			RenderServices::CreateIndices(context, indexCapacity, usage,
				static_cast<D3DFORMAT>(context.field0xAC), &indices);
		}
	}

	void GeometryStreams::Release()
	{
		if (vertices)
		{
			RenderServices::Release(vertices);
			vertices = NULL;
		}
		if (indices)
		{
			RenderServices::Release(indices);
			indices = NULL;
		}
	}

	HRESULT GeometryStreams::LockVertices(UINT count, UINT stride, BYTE** output, UINT* base,
		BYTE discard)
	{
		const UINT bytes = stride * count;
		UINT first = vertexPosition / stride + (vertexPosition % stride != 0);
		UINT offset = stride * first;
		DWORD flags = 6144;
		if (discard || count + first >= context.field0xA8 || bytes + offset > vertexCapacity)
		{
			first = 0;
			offset = 0;
			flags = 10240;
		}
		const HRESULT result = RenderServices::Lock(vertices, offset, bytes, output, flags);
		if (SUCCEEDED(result))
		{
			*base = first;
			vertexPosition = offset + bytes;
		}
		return result;
	}

	HRESULT GeometryStreams::LockIndices(UINT count, BYTE** output, UINT* base, BYTE discard)
	{
		UINT offset = indexBytes;
		UINT first = indexPosition;
		const UINT bytes = count * indexStride;
		DWORD flags = 6144;
		if (discard || bytes + offset >= indexCapacity)
		{
			first = 0;
			offset = 0;
			flags = 10240;
		}
		const HRESULT result = RenderServices::Lock(indices, offset, bytes, output, flags);
		if (SUCCEEDED(result))
		{
			*base = first;
			indexPosition = first + count;
			indexBytes = offset + bytes;
			context.statistics[5] += count;
		}
		return result;
	}

	void RenderQueue::Publish(UIMesh& mesh)
	{
		QueuedMesh current;
		current.mesh = &mesh;
		meshes.push_back(current);
	}

	void RenderQueue::Clear()
	{
		meshes.clear();
	}

	void RenderQueue::DrawPass(UINT pass, BYTE shadowOnly)
	{
		for (std::list<QueuedMesh>::iterator entry = meshes.begin(); entry != meshes.end(); ++entry)
		{
			QueuedMesh& current = *entry;
			UIMesh& mesh = *current.mesh;
			if (mesh.material.renderPass != pass)
				continue;
			const UINT vertexCount = static_cast<UINT>(mesh.vertices.size());
			BYTE* data;
			UINT vertexBase;
			if (FAILED(streams.LockVertices(vertexCount, sizeof(UIVertex), &data, &vertexBase)))
				continue;
			std::memcpy(data, mesh.vertices.empty() ? NULL : &mesh.vertices[0], vertexCount * sizeof(UIVertex));
			RenderServices::Unlock(streams.vertices);
			++context.statistics[9];
			RenderServices::Stream(context, streams.vertices, sizeof(UIVertex));
			current.vertexBase = vertexBase;
			if (!mesh.indices.empty())
			{
				const UINT indexCount = static_cast<UINT>(mesh.indices.size());
				UINT indexBase;
				if (SUCCEEDED(streams.LockIndices(indexCount, &data, &indexBase)))
				{
					std::memcpy(data, &mesh.indices[0], indexCount * streams.indexStride);
					RenderServices::Unlock(streams.indices);
					current.indexBase = indexBase;
					RenderServices::Indices(context, streams.indices, vertexBase);
				}
			}
			else
				current.indexBase = 0;
			RenderServices::Shader(context, 0x144);
			// Fresh native UI packets have the actual shadow-only flag clear.
			if (shadowOnly)
				continue;
			// UI packets have no Gfx_PacketData transform or light-owner override.
			context.SetTransform(D3DTS_WORLD, &context.identity);
			for (UINT light = 0; light < streams.activeLights; ++light)
				RenderServices::EnableLight(context, light, FALSE);
			context.statistics[4] += vertexCount;
			styles.SetMaterial(mesh.material, lighting, owner ? owner->fog : 0);
			UINT passes = 1;
			styles.Begin(&passes);
			for (UINT i = 0; i < passes; ++i)
			{
				styles.Pass(i);
				context.statistics[3] += mesh.primitiveCount;
				if (!mesh.indices.empty())
				{
					++context.statistics[12];
					RenderServices::DrawIndexed(context, mesh.primitive, 0, vertexCount,
						current.indexBase, mesh.primitiveCount);
				}
				else
				{
					++context.statistics[11];
					RenderServices::Draw(context, mesh.primitive, current.vertexBase,
						mesh.primitiveCount);
				}
			}
			styles.End();
		}
	}

	void RenderQueue::RenderEmptySkins(UINT pass)
	{
		if (pass == 0)
		{
			if (!context.currentDevice->vertexShaderSupported)
				context.SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, 1);
			context.SetRenderState(D3DRS_VERTEXBLEND, 0);
			context.SetRenderState(D3DRS_SHADEMODE, 1);
			context.SetRenderState(D3DRS_ZENABLE, 0);
			context.SetRenderState(D3DRS_ZWRITEENABLE, 0);
			context.SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
			context.SetRenderState(D3DRS_SRCBLEND, 5);
			context.SetRenderState(D3DRS_DESTBLEND, 6);
			context.SetRenderState(D3DRS_BLENDOP, 1);
			if (!context.currentDevice->vertexShaderSupported)
				context.SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, 0);
			context.SetRenderState(D3DRS_SHADEMODE, 2);
			context.SetRenderState(D3DRS_ZWRITEENABLE, 1);
			context.SetRenderState(D3DRS_ZENABLE, 1);
			context.SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
		}
		context.SetRenderState(D3DRS_VERTEXBLEND, 0);
	}

	void RenderQueue::Render(BYTE clear)
	{
		context.SetRenderState(D3DRS_ZENABLE, 1);
		context.SetRenderState(D3DRS_ZWRITEENABLE, 1);
		context.SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
		context.SetRenderState(D3DRS_ALPHAREF, 127);
		DrawPass(0, 0);
		DrawPass(0, 1);
		RenderEmptySkins(0);
		context.SetRenderState(D3DRS_ALPHAREF, 0);
		if (stencilShadows)
			throw std::logic_error("Unrecovered stencil shadow producer0x491FC0");
		if (floor)
			throw std::logic_error("Unrecovered RenderFloorCB0x4B94D0");
		context.SetRenderState(D3DRS_ZWRITEENABLE, 0);
		context.SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
		context.SetRenderState(D3DRS_SRCBLEND, 5);
		context.SetRenderState(D3DRS_DESTBLEND, 6);
		context.SetRenderState(D3DRS_BLENDOP, 1);
		RenderEmptySkins(1);
		DrawPass(1, 0);
		context.SetRenderState(D3DRS_SRCBLEND, 9);
		context.SetRenderState(D3DRS_DESTBLEND, 1);
		context.SetRenderState(D3DRS_BLENDOP, 1);
		RenderEmptySkins(2);
		DrawPass(2, 0);
		context.SetRenderState(D3DRS_SRCBLEND, 5);
		context.SetRenderState(D3DRS_DESTBLEND, 2);
		context.SetRenderState(D3DRS_BLENDOP, 2);
		RenderEmptySkins(3);
		DrawPass(3, 0);
		context.SetRenderState(D3DRS_SRCBLEND, 5);
		context.SetRenderState(D3DRS_DESTBLEND, 2);
		context.SetRenderState(D3DRS_BLENDOP, 3);
		RenderEmptySkins(4);
		DrawPass(4, 0);
		context.SetRenderState(D3DRS_SRCBLEND, 5);
		context.SetRenderState(D3DRS_DESTBLEND, 2);
		context.SetRenderState(D3DRS_BLENDOP, 1);
		RenderEmptySkins(5);
		DrawPass(5, 0);
		if (extraPass)
			throw std::logic_error("Unrecovered extra render pass0x491B10");
		if (clear)
			Clear();
	}

	void UIBatcher::BeginFrame()
	{
		for (unsigned i = 0; i < 4; ++i)
			pending[i].clear();
		generated.clear();
	}

	void UIBatcher::Submit(UIMesh& mesh, Viewport* target, UIBatchClass::Type kind)
	{
		if (target)
		{
			target->queue.Publish(mesh);
			return;
		}
		const unsigned index = static_cast<unsigned>(kind);
		if (index >= 4)
			return; // Original switch has no action for an unrecognized batch class.
		std::list<UIMesh*>& list = pending[index];
		if (kind == UIBatchClass::OpaqueQuads || kind == UIBatchClass::Text)
		{
			const DWORD factor = kind == UIBatchClass::Text ? MaterialFactor(mesh.material) : 0;
			std::list<UIMesh*>::iterator position = list.begin();
			for (; position != list.end(); ++position)
			{
				UIMesh* item = *position;
				if (item->material.texture == mesh.material.texture &&
					(kind != UIBatchClass::Text || MaterialFactor(item->material) == factor)) break;
			}
			list.insert(position, &mesh);
		}
		else
			list.push_back(&mesh);
	}

	void UIBatcher::FlushQuads()
	{
		std::list<UIMesh*>::iterator first = pending[0].begin();
		while (first != pending[0].end())
		{
			std::list<UIMesh*>::iterator last = first;
			IDirect3DBaseTexture8* texture = (*first)->material.texture;
			UINT count = 0;
			do
			{
				++count;
				++last;
			} while (last != pending[0].end() && (*last)->material.texture == texture);
			generated.push_back(UIMesh());
			UIMesh& batch = generated.back();
			batch.material.style = 9;
			batch.material.renderPass = 0;
			batch.material.texture = texture;
			batch.primitive = D3DPT_TRIANGLELIST;
			batch.primitiveCount = 2 * count;
			batch.vertices.resize(4 * count);
			batch.indices.resize(6 * count);
			for (UINT i = 0; i < count; ++i)
			{
				const WORD base = static_cast<WORD>(4 * i);
				const WORD indices[] = {base, WORD(base + 1), WORD(base + 2),
					WORD(base + 2), WORD(base + 1), WORD(base + 3)};
				std::copy(indices, indices + 6, batch.indices.begin() + 6 * i);
				std::memcpy(&batch.vertices[4 * i], &(*first)->vertices[0], 112);
				++first;
			}
			viewport.queue.Publish(batch);
		}
	}

	void UIBatcher::FlushText()
	{
		std::list<UIMesh*>::iterator first = pending[3].begin();
		while (first != pending[3].end())
		{
			IDirect3DBaseTexture8* texture = (*first)->material.texture;
			const DWORD factor = MaterialFactor((*first)->material);
			std::list<UIMesh*>::iterator last = first;
			UINT vertices = 0;
			do
			{
				vertices += static_cast<UINT>((*last)->vertices.size());
				++last;
			} while (last != pending[3].end() && (*last)->material.texture == texture &&
				MaterialFactor((*last)->material) == factor);
			generated.push_back(UIMesh());
			UIMesh& batch = generated.back();
			batch.material.texture = texture;
			batch.primitive = D3DPT_TRIANGLELIST;
			batch.primitiveCount = vertices / 3;
			batch.vertices.reserve(vertices);
			if (factor != 0xFF808080u)
				SetMaterialFactor(batch.material, factor);
			while (first != last)
			{
				const std::vector<UIVertex>& source = (*first)->vertices;
				batch.vertices.insert(batch.vertices.end(), source.begin(), source.end());
				++first;
			}
			viewport.queue.Publish(batch);
		}
	}

	void UIBatcher::Flush()
	{
		std::list<UIMesh*>::iterator entry;
		for (entry = pending[1].begin(); entry != pending[1].end(); ++entry)
			viewport.queue.Publish(**entry);
		FlushQuads();
		FlushText();
		for (entry = pending[2].begin(); entry != pending[2].end(); ++entry)
			viewport.queue.Publish(**entry);
	}

	namespace RenderServices
	{
		HRESULT CreateVertices(D3DContext& c, UINT size, DWORD usage, IDirect3DVertexBuffer8** out)
		{ return c.device->CreateVertexBuffer(size, usage, 0, D3DPOOL_DEFAULT, out); }
		HRESULT CreateIndices(D3DContext& c, UINT size, DWORD usage, D3DFORMAT format,
			IDirect3DIndexBuffer8** out)
		{ return c.device->CreateIndexBuffer(size, usage, format, D3DPOOL_DEFAULT, out); }
		ULONG Release(IDirect3DVertexBuffer8* buffer) { return buffer->Release(); }
		ULONG Release(IDirect3DIndexBuffer8* buffer) { return buffer->Release(); }
		HRESULT Lock(IDirect3DVertexBuffer8* b, UINT at, UINT size, BYTE** out, DWORD flags)
		{ return b->Lock(at, size, out, flags); }
		HRESULT Lock(IDirect3DIndexBuffer8* b, UINT at, UINT size, BYTE** out, DWORD flags)
		{ return b->Lock(at, size, out, flags); }
		HRESULT Unlock(IDirect3DVertexBuffer8* buffer) { return buffer->Unlock(); }
		HRESULT Unlock(IDirect3DIndexBuffer8* buffer) { return buffer->Unlock(); }
		HRESULT Stream(D3DContext& c, IDirect3DVertexBuffer8* buffer, UINT stride)
		{ return c.device->SetStreamSource(0, buffer, stride); }
		HRESULT Indices(D3DContext& c, IDirect3DIndexBuffer8* buffer, UINT base)
		{ return c.device->SetIndices(buffer, base); }
		HRESULT Shader(D3DContext& c, DWORD fvf) { return c.device->SetVertexShader(fvf); }
		HRESULT EnableLight(D3DContext& c, UINT index, BOOL enable)
		{ return c.device->LightEnable(index, enable); }
		HRESULT Draw(D3DContext& c, D3DPRIMITIVETYPE primitive, UINT start, UINT count)
		{ return c.device->DrawPrimitive(primitive, start, count); }
		HRESULT DrawIndexed(D3DContext& c, D3DPRIMITIVETYPE primitive, UINT minimum,
			UINT vertices, UINT start, UINT count)
		{ return c.device->DrawIndexedPrimitive(primitive, minimum, vertices, start, count); }
	}
}
}
