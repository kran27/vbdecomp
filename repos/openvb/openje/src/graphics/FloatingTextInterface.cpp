#include "graphics/FloatingTextInterface.hpp"
#include "gui/Event.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include "model/ModelInstance.hpp"
#include "graphics/Camera.hpp"
#include "graphics/SystemInterface.hpp"
#include <mmsystem.h>
#include <stdexcept>

namespace JE
{
	FloatingTextConfiguration::FloatingTextConfiguration()
	{
		profile = "DefaultInterface.ini";
		font = "JH_FALLOUT";
		red = 128;
		green = 128;
		blue = 128;
		pointSize = 10;
		fontFlags = 0;
		duration = 1000;
		speed = 0;
		fadeOut = 0;
		fadeIn = 0;
		width = 200;
		height = 11;
		wrap = 0;
		alignment = 0;
		backgroundProfile = "DefaultInterface.ini";
		paddingLeft = 6;
		paddingRight = 6;
		paddingTop = 4;
		paddingBottom = 4;
		backgroundAlpha = 180;
	}

	FloatingTextInterface::FloatingTextInterface(const std::string& name) : EngineInterface(name)
	{
		activeLabels.clear();
		for (int i = 0; i < 32; ++i)
		{
			::GUI::Window& background = labels[i].background;
			AddWindow(&labels[i]);
			labels[i].SetHidden(1); labels[i].duration = 0;
			labels[i].AddChild(&background);
			background.SetValue3B(1); background.SetHidden(1);
		}
	}
	FloatingTextInterface::~FloatingTextInterface()
	{
		// Embedded labels destruct/unlink before EngineInterface's root cleanup.
		// Normal member destruction then clears active nodes and configurations.
	}
	void FloatingTextInterface::Clear() //58FEE0.
	{
		std::list<::GUI::FloatingLabel*>::iterator entry = activeLabels.begin();
		while (entry != activeLabels.end())
		{
			::GUI::FloatingLabel* label = *entry;
			if (label)
			{
				entry = activeLabels.erase(entry);
				label->SetHidden(1);
				label->background.SetHidden(1);
				label->duration = 0;
			}
			//58FF5A follows the next link even after erase. Preserve the
			//original sentinel wrap and skipped next nonempty entry.
			++entry;
		}
	}
	::GUI::FloatingLabel* FloatingTextInterface::FindFree()
	{
		for (DWORD i = 0; i < 32; ++i) if (labels[i].GetHidden()) return &labels[i];
		return 0;
	}
	void FloatingTextInterface::Add(DWORD id, const std::string& text, DWORD red, DWORD green, DWORD blue,
		const std::string& profile, const std::string& font, LONG pointSize, DWORD fontFlags,
		DWORD duration, DWORD speed, DWORD fadeOut, DWORD fadeIn, LONG width, LONG height,
		BYTE wrap, DWORD alignment, DWORD kind)
	{
		Entity* entity = GetEntityPtr(id);
		IWorldClient* world = g_World;
		const DWORD mapID = world->GetCurrentMapID();
		if (!entity || entity->mapHandle != mapID) return;
		::GUI::FloatingLabel* label = FindFree();
		if (!label) return;
		label->SetHidden(0);
		label->ApplySkin(profile);
		label->SetFont(font, fontFlags, pointSize, 1);
		label->SetTextColor(0xFF000000 | ((red & 255) << 16) | ((green & 255) << 8) | (blue & 255));
		label->SetWrap(wrap);
		label->SetAlignment(alignment);
		if (wrap)
		{
			label->Resize(width, 768);
			label->SetText(text);
			const LONG textHeight = label->GetTextHeight() + 1;
			label->Resize(width, textHeight);
			if (kind == 1)
			{
				const LONG textWidth = label->GetTextWidth(); LONG x, y;
				::GUI::Window& background = label->background;
				label->GetAbsolutePosition(x, y);
				// Kind1 uses the live speech background configuration; Damage is kind2.
				background.ApplySkin(speech.backgroundProfile);
				const DWORD alpha = speech.backgroundAlpha;
				label->backgroundAlpha = static_cast<BYTE>(alpha);
				background.SetAlpha(static_cast<BYTE>(alpha));
				background.SetAbsolutePosition(x - speech.paddingLeft, y - speech.paddingTop);
				background.Resize(textWidth + speech.paddingLeft + speech.paddingRight,
					textHeight + speech.paddingTop + speech.paddingBottom);
				background.SetHidden(0);
			}
		}
		else { label->SetText(text); label->Resize(width, height); }
		label->kind = kind; label->entityID = id; label->birthTime = timeGetTime();
		label->duration = duration; label->speed = speed;
		label->fadeOut = fadeOut; label->fadeIn = fadeIn;
		activeLabels.push_front(label);
	}
	DWORD FloatingTextInterface::GetReadingTime(const std::string& text)
	{
		return static_cast<DWORD>((text.size() * 0.1f + 1.0f) * 1000.0f);
	}
	void FloatingTextInterface::AfterPosition(::GUI::FloatingLabel*, LONG, LONG, LONG, LONG) {}
	int FloatingTextInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (event->type == 0 && window == &labels[0]) UpdatePositions();
		return EngineInterface::OnEvent(window, event);
	}
	void FloatingTextInterface::UpdatePositions()
	{
		if (activeLabels.empty()) return;
		const DWORD now = timeGetTime();
		std::list<::GUI::FloatingLabel*>::iterator entry = activeLabels.begin();
		while (entry != activeLabels.end())
		{
			::GUI::FloatingLabel* label = *entry;
			if (label)
			{
				if (now > label->birthTime + label->duration)
				{
					entry = activeLabels.erase(entry);
					label->SetHidden(1); label->background.SetHidden(1); label->duration = 0;
				}
				else
				{
					Entity* entity = GetEntityPtr(label->entityID);
					if (entity)
					{
						D3DXVECTOR3 position;
						position.x = entity->position.x;
						position.y = entity->position.y;
						position.z = entity->position.z;
						const double height = entity->GetModelHeight();
						D3DXVECTOR3 point(position.x, float(position.y + height), position.z);
						D3DXMATRIX view, projection, combined;
						g_GfxSystem->Camera()->GetViewMatrix(view);
						g_GfxSystem->Camera()->GetProjectionMatrix(projection);
						D3DXMatrixMultiply(&combined, &view, &projection);
						D3DXVECTOR4 clip; D3DXVec3Transform(&clip, &point, &combined);
						const float inverseW = 1.0f / clip.w;
						const float screenX = static_cast<float>((clip.x * inverseW + 1.0f) * 512.0f);
						const float screenY = static_cast<float>((2.0f - (clip.y * inverseW + 1.0f)) * 384.0f);
						LONG width, labelHeight; label->GetSize(width, labelHeight);
						const LONG y = static_cast<LONG>(screenY), x = static_cast<LONG>(screenX);
						label->MoveAtScreen(x, y, width, labelHeight, now);
						AfterPosition(label, x, y, width, labelHeight);
					}
				}
			}
			// Follow the next node after either live processing or erase, including sentinel wrap.
			++entry;
		}
	}
}
