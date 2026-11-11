#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "gui/DialogueWindow.hpp"
#include "gui/DialogueInterface.hpp"
#include "gui/ReplyLabel.hpp"
#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "entity/Entity.hpp"
#include "model/ModelInstance.hpp"
#include "graphics/ModelInstance.hpp"
#include "graphics/AnimationObject.hpp"
#include "graphics/AnimationInstance.hpp"
#include "graphics/Viewport.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Camera.hpp"
#include "resources/ResourceMemoryStream.hpp"
#include "network/Packet.hpp"
#include "gui/Interfaces.hpp"
#include "startup/EngineStringSystem.hpp"
#include "game/CombatRandom.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <mmsystem.h>
#include <stdexcept>

namespace JE
{
	namespace
	{
		bool InvalidParticipant(Entity* entity)
		{
			return !entity || entity->moving == 1 || !entity->renderEnabled ||
				entity->IsDead() == 1 || !entity->IsVisible();
		}
		int Different(const DialogueWindowMember& a, const DialogueWindowMember& b) //5929E0; -1 differs,0 equal.
		{
			if (a.textID != b.textID) return -1;
			const LONG replyCount = a.replyCount;
			if (replyCount != b.replyCount) return -1;
			if (a.replies != b.replies) return -1;
			for (LONG i = 0; i < replyCount; ++i)
				if (a.replyOrder[i] != b.replyOrder[i]) return -1;
			return 0;
		}
		DWORD Scaled(LONG value, float scale)
		{
			return DWORD(static_cast<LONGLONG>(double(value) * scale));
		}
	}

	void DialogueWindow::SetParticipants(DWORD listener, DWORD speaker) //595010.
	{
		Net::Packet close;
		Entity* speakerEntity = GetEntityPtr(speaker);
		if (InvalidParticipant(speakerEntity))
		{
			close.WriteByte(dialogueID);
			DialogueWindowServices::RequestClose(close);
		}
		Entity* listenerEntity = GetEntityPtr(listener);
		if (InvalidParticipant(listenerEntity))
		{
			close.WriteByte(dialogueID);
			DialogueWindowServices::RequestClose(close);
		}
		std::map<DWORD, DialogueWindowMember>::iterator found = members.find(listener);
		if (found == members.end())
			throw std::logic_error("DialogueWindow595203: original unchecked missing member");
		DialogueWindowMember member(found->second);
		const BYTE changed = listenerID != listener || !currentMember || Different(member, *currentMember);
		if (!currentMember) currentMember = new DialogueWindowMember;
		*currentMember = member;
		listenerID = listener;
		std::string text;
		if (changed && member.textID) F3::g_EngineStringSystem.GetString(member.textID, text);
		std::string formatted;

		if (speakerID != speaker)
		{
			if (currentMember) { delete currentMember; currentMember = NULL; }
			if (portraitModel) { delete portraitModel; portraitModel = NULL; }
			if (portraitModelDataHandle) g_GfxSystem->ReleaseModelData(&portraitModelDataHandle);
			if (portraitAnimation) { delete portraitAnimation; portraitAnimation = NULL; }
			portraitAnimationObject->ClearAnimationGroups();
			lastAnimationName.clear();
			speakerID = speaker;
		}

		LONG width, height, totalWidth, totalHeight;
		const DialogueAppearance& s = dialogueInterface->speaker;
		const DialogueAppearance& l = dialogueInterface->listener;
		if (changed)
		{
			LONG textWidth, textHeight;
			speakerText->GetSize(textWidth, textHeight);
			speakerText->Resize(textWidth, 768);
			FormatLine(formatted, text, listenerID, 0);
			speakerText->SetText(formatted);
			textHeight = speakerText->GetTextHeight() + 1;
			speakerText->Resize(textWidth, textHeight);

			if (portraitAnimationObject)
			{
				char name[64];
				std::sprintf(name, "HumF_Def_1H_Fidget_0%d.gr2", F3::CombatRandomRange(1, 1));
				if (_stricmp(lastAnimationName.c_str(), name) != 0)
				{
					Resources::ResourceMemoryStream* stream = Resources::OpenResourceMemoryStream(name, 0);
					if (stream)
					{
						portraitAnimationObject->DestroyAnimationGroup(2);
						lastAnimationName.clear();
						if (portraitAnimationObject->LoadAnimationFileFromBuffer(2,
							int(stream->end - stream->begin), stream->begin, 10.0f))
							lastAnimationName = name;
						Resources::CloseResourceMemoryStream(stream);
					}
				}
			}
			if (portraitAnimation)
			{
				portraitAnimation->SelectAnimation(2, 0, 0, 0.0f, 1, 1);
				portraitAnimation->SetSpeed(1.0f);
				portraitAnimation->reverse = 0;
				animationTimeOrigin = timeGetTime();
			}

			GetSize(width, height);
			height = textHeight + s.textLabelY + s.textBottomMargin;
			const LONG minimum = s.modelHeight + 2 * s.modelY;
			if (height < minimum) height = minimum;
			Resize(width, height);
			LONG x, y;
			GetAbsolutePosition(x, y);
			listenerWindow->SetAbsolutePosition(x, y + height);
			totalWidth = width;
			totalHeight = height;
			for (int i = 0; i < 10; ++i)
			{
				replies[i]->SetText(std::string());
				replies[i]->SetReplyID(DWORD(-1));
				replies[i]->SetHidden(1);
			}
			LONG rowY = 0;
			for (LONG i = 0; i < member.replyCount; ++i)
			{
				const DWORD replyID = member.replyOrder[i];
				replies[i]->SetReplyID(replyID);
				std::map<DWORD, DWORD>::const_iterator reply = member.replies.find(replyID);
				if (reply == member.replies.end())
					throw std::logic_error("DialogueWindow595780: original unchecked reply map miss");
				replies[i]->GetSize(textWidth, textHeight);
				replies[i]->Resize(textWidth, 768);
				F3::g_EngineStringSystem.GetString(reply->second, text);
				FormatLine(formatted, text, listenerID, i + 1);
				replies[i]->SetText(formatted);
				textHeight = replies[i]->GetTextHeight() + 1;
				replies[i]->Resize(textWidth, textHeight);
				listenerWindow->GetAbsolutePosition(x, y);
				replies[i]->SetAbsolutePosition(x + l.textLabelX, y + l.textLabelY + rowY);
				replies[i]->SetHidden(0);
				rowY += textHeight + LONG(l.textValue100);
			}
			listenerWindow->GetSize(width, height);
			height = rowY + l.textLabelY + l.textBottomMargin;
			listenerWindow->Resize(width, height);
			if (width > totalWidth) totalWidth = width;
			totalHeight += height;
		}
		else
		{
			GetSize(width, height);
			totalWidth = width;
			totalHeight = height;
			listenerWindow->GetSize(width, height);
			if (width > totalWidth) totalWidth = width;
			totalHeight += height;
		}

		if (!speakerEntity)
			throw std::logic_error("DialogueWindow59591F: original NULL speaker after close request");
		D3DXVECTOR3 head = speakerEntity->position;
		double modelHeight = 0.0;
		if (speakerEntity->model)
		{
			D3DXVECTOR3 minimum, maximum;
			speakerEntity->model->GetBounds(minimum, maximum);
			modelHeight = double(maximum.y) - minimum.y;
		}
		head.y = float(double(head.y) + modelHeight);
		D3DXMATRIX view, projection, combined;
		g_GfxSystem->Camera()->GetViewMatrix(view);
		g_GfxSystem->Camera()->GetProjectionMatrix(projection);
		D3DXMatrixMultiply(&combined, &view, &projection);
		D3DXVECTOR4 clip;
		D3DXVec3Transform(&clip, &head, &combined);
		const double inverseW = 1.0 / double(clip.w);
		//Raw SDK cleanup resolves decompiler's width alias: actual totalWidth,
		//not the last reply height. Y is converted first, then X.
		const LONG screenY = LONG(static_cast<LONGLONG>((2.0 - (inverseW * clip.y + 1.0)) * 384.0));
		const LONG screenX = LONG(static_cast<LONGLONG>((clip.x * inverseW + 1.0) * 512.0));
		PositionToSpeaker(screenX, screenY, totalWidth, totalHeight);
	}

	void DialogueWindow::PositionToSpeaker(LONG x, LONG y, LONG width, LONG height) //5921B0.
	{
		Net::Packet close;
		y -= height;
		LONG screenX, screenY, screenWidth, screenHeight;
		static_cast<Client*>(F3::g_pNetworkInstance->client)->child0xA0->GetScreenRectangle(
			screenX, screenY, screenWidth, screenHeight);
		if (x + width < 0 || x > screenWidth || y + height < 0 || y > screenHeight)
		{
			close.WriteByte(dialogueID);
			static_cast<Client*>(F3::g_pNetworkInstance->client)->SendDialogueClose(close);
		}
		else
		{
			if (x < 0) x = 0;
			if (x + width > screenWidth) x = screenWidth - width;
			if (y < 0) y = 0;
			if (y + height > screenHeight) y = screenHeight - height;
			SetAbsolutePosition(x, y);
			float sx, sy;
			::GUI::g_Manager.GetScale(sx, sy);
			LONG modelWidth, modelHeight;
			portraitWindow->GetAbsolutePosition(screenX, screenY);
			portraitWindow->GetSize(modelWidth, modelHeight);
			portraitViewport->SetViewport(Scaled(screenX, sx), Scaled(screenY, sy),
				Scaled(modelWidth, sx), Scaled(modelHeight, sy), 0.0f, 1.0f);
		}
	}
}
