#include "MenuFragment.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace JE
{
	namespace GUI
	{
		namespace
		{
			LONG Signed(DWORD value)
			{
				LONG result;
				std::memcpy(&result, &value, sizeof(result));
				return result;
			}

			LONG Difference(LONG left, LONG right)
			{
				return Signed(static_cast<DWORD>(left) - static_cast<DWORD>(right));
			}

			LONG Sum(LONG left, LONG right)
			{
				return Signed(static_cast<DWORD>(left) + static_cast<DWORD>(right));
			}

			double SourceSize(float value, LONG texture)
			{
				const double size = value < 0 ? static_cast<double>(texture) : value;
				return size < 0 ? 0 : size;
			}

			void SubmitFragment(MenuFragment& fragment, FragmentServices& services,
				Gfx::Viewport* target, BYTE kind, bool batch)
			{
				if (!fragment.geometryAvailable)
					throw std::logic_error("Fragment positions have not been initialized by GUI layout");
				services.Submit(fragment.mesh, target, kind, batch);
			}
		}

		MenuFragment::MenuFragment() : geometryDirty(1), uvDirty(1), tileMode(0),
			frameWidth(2), frameHeight(2), frameRate(0), currentFrame(0), sourceX(0),
			sourceY(0), sourceWidth(-1), sourceHeight(-1), color(0xFF808080u),
			geometryAvailable(false)
		{
			rectangle.left = rectangle.top = 0;
			rectangle.right = rectangle.bottom = 64;
			// Original 0x4B24A0's vertex subobject and material defaults only.
			// Complete fragment mesh/manager lifetime remains a separate unit.
			mesh.vertices.resize(4);
			for (unsigned i = 0; i < 4; ++i)
			{
				Gfx::UIVertex& vertex = mesh.vertices[i];
				vertex.z = 0;
				vertex.rhw = 1;
				vertex.diffuse = 0xFF808080u;
				vertex.u = (i & 1) ? 1.0f : 0.0f;
				vertex.v = i >= 2 ? 1.0f : 0.0f;
			}
			mesh.material.style = 20;
			mesh.material.renderPass = 1;
		}

		void MenuFragment::UpdateUV(FragmentServices& services)
		{
			// 0x4B1B00 clears this flag before resolving the texture.
			uvDirty = 0;
			const FragmentTextureInfo* info = mesh.material.texture ?
				services.TextureInfo(mesh.material.texture) : NULL;
			double u0 = 0, v0 = 0, u1 = 1, v1 = 1;
			if (info)
			{
				const LONG width = Difference(rectangle.right, rectangle.left);
				const LONG height = Difference(rectangle.bottom, rectangle.top);
				const float sourceW = static_cast<float>(SourceSize(sourceWidth, info->width));
				const float sourceH = static_cast<float>(SourceSize(sourceHeight, info->height));
				const float textureW = static_cast<float>(info->width);
				const float textureH = static_cast<float>(info->height);
				if (!tileMode || (tileMode & 4))
				{
					double selectedW = sourceW, selectedH = sourceH;
					if (tileMode & 4)
					{
						float scaleX, scaleY;
						services.Scale(scaleX, scaleY); // Original call, even though unused here.
						if (selectedW >= width) selectedW = width;
						if (selectedH >= height) selectedH = height;
					}
					const float origin = static_cast<float>(
						static_cast<double>(Signed(frameWidth * currentFrame)) + sourceX);
					u0 = static_cast<double>(origin) / textureW;
					v0 = static_cast<double>(sourceY) / textureH;
					const float roundedU = static_cast<float>((origin + selectedW) / textureW);
					const float roundedV = static_cast<float>((selectedH + sourceY) / textureH);
					u1 = roundedU;
					v1 = roundedV;
				}
				else if (tileMode == 1)
				{
					v0 = static_cast<double>(Signed(currentFrame)) * height / textureH;
					u1 = static_cast<float>(static_cast<double>(width) / textureW);
					v1 = static_cast<float>(static_cast<double>(Signed(currentFrame + 1)) * height / textureH);
				}
				else if (tileMode == 2)
				{
					u0 = static_cast<double>(Signed(currentFrame)) * width / textureW;
					u1 = static_cast<float>(static_cast<double>(Signed(currentFrame + 1)) * width / textureW);
					v1 = static_cast<float>(static_cast<double>(height) / textureH);
				}
				else if (tileMode == 3)
				{
					u1 = static_cast<float>(static_cast<double>(width) / textureW);
					v1 = static_cast<float>(static_cast<double>(height) / textureH);
				}
				else
				{
					throw std::logic_error("Fragment 0x4B1B00 consumes unwritten UV locals for this tile mode");
				}
			}
			mesh.vertices[0].u = mesh.vertices[2].u = static_cast<float>(u0);
			mesh.vertices[0].v = mesh.vertices[1].v = static_cast<float>(v0);
			mesh.vertices[1].u = mesh.vertices[3].u = static_cast<float>(u1);
			mesh.vertices[2].v = mesh.vertices[3].v = static_cast<float>(v1);
		}

		void MenuFragment::UpdateGeometry(FragmentServices& services)
		{
			geometryDirty = 0; // 0x4B2890.
			float scaleX, scaleY;
			services.Scale(scaleX, scaleY);
			LONG right = rectangle.right, bottom = rectangle.bottom;
			double leftPosition, topPosition, rightPosition, bottomPosition;
			if (tileMode & 4)
			{
				double selectedW = 0, selectedH = 0;
				if (mesh.material.texture)
				{
					const FragmentTextureInfo* info = services.TextureInfo(mesh.material.texture);
					if (!info) throw std::logic_error("Fragment geometry requires its original nonnull texture metadata");
					selectedW = static_cast<float>(SourceSize(sourceWidth, info->width));
					selectedH = SourceSize(sourceHeight, info->height);
				}
				const LONG width = Difference(right, rectangle.left);
				const LONG height = Difference(bottom, rectangle.top);
				if (selectedW >= width) selectedW = width;
				const float roundedHeight = static_cast<float>(height);
				if (selectedH >= roundedHeight) selectedH = roundedHeight;
				right = Sum(rectangle.left, static_cast<LONG>(selectedW));
				bottom = Sum(rectangle.top, static_cast<LONG>(selectedH));
				leftPosition = static_cast<LONG>(static_cast<double>(rectangle.left) * scaleX);
				topPosition = static_cast<LONG>(static_cast<double>(rectangle.top) * scaleY);
				rightPosition = static_cast<LONG>(Difference(right, rectangle.left) + leftPosition);
				const float roundedTop = static_cast<float>(topPosition);
				bottomPosition = static_cast<LONG>(Difference(bottom, rectangle.top) + static_cast<double>(roundedTop));
			}
			else
			{
				const float halfX = static_cast<float>(static_cast<double>(scaleX) * 0.5);
				const float halfY = static_cast<float>(static_cast<double>(scaleY) * 0.5);
				leftPosition = static_cast<double>(rectangle.left) * scaleX - halfX;
				topPosition = static_cast<double>(rectangle.top) * scaleY - halfY;
				rightPosition = (static_cast<double>(right) - 1.0) * scaleX + halfX;
				bottomPosition = static_cast<double>(Difference(bottom, 1)) * scaleY + halfY;
			}
			mesh.vertices[0].x = mesh.vertices[2].x = static_cast<float>(leftPosition);
			mesh.vertices[0].y = mesh.vertices[1].y = static_cast<float>(topPosition);
			mesh.vertices[1].x = mesh.vertices[3].x = static_cast<float>(rightPosition);
			mesh.vertices[2].y = mesh.vertices[3].y = static_cast<float>(bottomPosition);
			geometryAvailable = true;
		}

		void MenuFragment::Draw(FragmentServices& services, Gfx::Viewport* target, BYTE flags)
		{
			if (frameRate)
			{
				const DWORD now = services.TimeMS();
				const DWORD elapsed = now - lastFrameTime;
				const DWORD interval = 1000 / frameRate;
				if (elapsed > interval)
				{
					// Keep original divide-by-zero domain explicit; do not fabricate timing.
					if (!interval || !frameCount)
						throw std::logic_error("Fragment animation requires nonzero interval and frame count");
					lastFrameTime = now;
					uvDirty = 1;
					currentFrame = (currentFrame + elapsed / interval) % frameCount;
				}
			}
			if (uvDirty) UpdateUV(services);
			if (!services.BatchingEnabled())
			{
				SubmitFragment(*this, services, target, 1, false);
				return;
			}
			const BYTE alpha = static_cast<BYTE>(color >> 24);
			if (alpha == 255)
			{
				const BYTE kind = (flags & 1) ? ((flags & 2) ? 2 : 1) : 0;
				SubmitFragment(*this, services, target, kind, true);
			}
			else if (alpha)
			{
				SubmitFragment(*this, services, target, (flags & 2) ? 2 : 1, true);
			}
		}
	}
}
