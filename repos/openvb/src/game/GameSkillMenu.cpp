#include <winsock2.h>
#include "game/GameSkillMenu.hpp"
#include "game/GameEntity.hpp"
#include "game/GameDoor.hpp"
#include "game/GameContainer.hpp"
#include "game/GameCreature.hpp"
#include "game/GameActionOperateDoor.hpp"
#include "game/GameActionOperateContainer.hpp"
#include "game/ClientDialogueActions.hpp"
#include "game/ClientUseRequests.hpp"
#include "game/GameInterface.hpp"
#include "game/ClientSkillActions.hpp"
#include "startup/EngineStringSystem.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Camera.hpp"
#include "network/EngineClient.hpp"
#include "scripting/ScriptSmall.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "gui/Event.hpp"
#include "gui/Interfaces.hpp"
#include "gui/WorldContainerServices.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

namespace
{
	typedef JE::ContextMenuEntry SkillMenuEntry;

}

namespace JE
{
	void Client::RequestSkillMenu(DWORD actorID, DWORD targetID, LONG x, LONG y)
	{
		Entity* actor = GetEntityPtr(actorID);
		F3::GameEntity* target = dynamic_cast<F3::GameEntity*>(GetEntityPtr(targetID));
		F3::GameSkillMenu* menu = dynamic_cast<F3::GameSkillMenu*>(child0xB0);
		if (actor && target && menu && target->ShowSkills())
			menu->Open(actorID, targetID, x, y);
	}

	void Client::RequestContextMenu(DWORD actorID, DWORD targetID, LONG x, LONG y)
	{
		child0xB0->Show(actorID, targetID, x, y, 4000, 4000);
	}

	void ContextMenuEntry::SetStringReference(DWORD value)
	{
		stringReference = value;
	}

	CSMenuLabel::CSMenuLabel() : ::GUI::Label(3), selector(0), valueE68(0),
		valueE6C(0), actorID(0), targetID(0)
	{
		SetName("CSMenuLabel");
		ApplySkin("DefaultInterface.ini");
		SetFont("Arial", 0, 9, 1);
		SetTextColor(0xFF808080);
		SetValue3B(0);
		disabled = 0; //568E2E: after the original virtual setup callbacks.
	}

	CSMenuLabel::~CSMenuLabel()
	{
	}

	BOOL CSMenuLabel::OnEvent(::GUI::Event* event)
	{
		if (!disabled)
		{
			if (event->type == 1) SetTextColor(0xFF808000);
			else if (event->type == 3) SetTextColor(0xFF808080);
		}
		return ::GUI::Window::OnEvent(event);
	}

	void CSMenuLabel::SetDisabled(BYTE value)
	{
		disabled = value;
		if (value) SetTextColor(0xFF404040);
		else SetTextColor(0xFF808080);
	}

	CSMenuInterface::CSMenuInterface(const std::string& name)
		: EngineInterface(name), rootWindow(0), anchor(0, 0, 0)
	{
		rootWindow.ApplySkin("DefaultInterface.ini");
		rootWindow.SetName("CSMenu");
		rootWindow.SetAlpha(180);
		rootWindow.SetHidden(1);
		AddWindow(&rootWindow);
		maximumTextWidth = 0;
		labels.clear();
		fadeStart = 0;
		fadeDuration = 0;
		actorID = 0;
		targetID = 0;
	}

	CSMenuInterface::~CSMenuInterface()
	{
		Clear();
	}

	void CSMenuInterface::Populate(DWORD, DWORD) {} //Original408090 empty primary hook.

	void CSMenuInterface::Clear()
	{
		for (std::list<CSMenuLabel*>::iterator i = labels.begin(); i != labels.end(); ++i)
			if (*i) delete *i;
		labels.clear();
		maximumTextWidth = 0;
	}

	void CSMenuInterface::Close()
	{
		fadeStart = 0;
		GetRoots()->front()->SetAlpha(255);
		rootWindow.SetHidden(1);
	}

	void CSMenuInterface::Fade(DWORD time)
	{
		const DWORD duration = fadeDuration;
		const DWORD elapsed = time - fadeStart;
		if (elapsed >= duration)
		{
			Close();
			return;
		}
		const DWORD alpha = DWORD(static_cast<LONGLONG>(
			255.0 - float(elapsed) / float(duration) * 255.0));
		GetRoots()->front()->SetAlpha(BYTE(alpha));
	}

	CSMenuLabel* CSMenuInterface::AddLabel(const std::string& text, DWORD actor,
		DWORD target, DWORD selector, DWORD valueE68, DWORD valueE6C, BYTE disabled)
	{
		const DWORD row = DWORD(labels.size());
		CSMenuLabel* label = new CSMenuLabel;
		label->SetText(text);
		label->SetTextColor(0xFF808080);
		label->actorID = actor;
		label->targetID = target;
		label->selector = selector;
		label->valueE68 = valueE68;
		label->valueE6C = valueE6C;
		rootWindow.AddChild(label);
		label->Resize(400, 16);
		label->SetClientPosition(8, LONG(16 * row + 6));
		if (disabled) label->SetDisabled(1);
		labels.push_back(label);
		if (maximumTextWidth < label->GetTextWidth())
			maximumTextWidth = label->GetTextWidth();
		return label;
	}

	CSMenuLabel* CSMenuInterface::FindSkillLabel(DWORD selector) const
	{
		for (std::list<CSMenuLabel*>::const_iterator i = labels.begin(); i != labels.end(); ++i)
			if (*i && (*i)->selector == selector) return *i;
		return NULL;
	}

	void CSMenuInterface::Show(DWORD actor, DWORD target, LONG x, LONG y,
		DWORD delay, DWORD duration)
	{
		Clear();
		Populate(actor, target);
		rootWindow.SetAbsolutePosition(x, y);
		rootWindow.Resize(LONG(DWORD(maximumTextWidth) + 24), LONG(16 * labels.size() + 12));
		for (std::list<CSMenuLabel*>::iterator i = labels.begin(); i != labels.end(); ++i)
			if (*i) (*i)->Resize(maximumTextWidth, 16);
		anchor = GetEntityPtr(target)->position; //Original unchecked target after Populate.
		fadeStart = GetWorldInstance()->GetTime() + delay;
		fadeDuration = duration;
		actorID = actor;
		targetID = target;
		rootWindow.SetHidden(0);
	}

	void CSMenuInterface::BeforeUpdate()
	{
		D3DXVECTOR3 position;
		position.x = anchor.x;
		position.y = anchor.y;
		position.z = anchor.z;
		D3DXMATRIX view, projection, combined;
		g_GfxSystem->Camera()->GetViewMatrix(view);
		g_GfxSystem->Camera()->GetProjectionMatrix(projection);
		D3DXMatrixMultiply(&combined, &view, &projection);
		D3DXVECTOR4 clip;
		D3DXVec3Transform(&clip, &position, &combined);
		LONG screenLeft, screenTop, screenWidth = 1024, screenHeight = 768;
		const float inverseW = 1.0f / clip.w;
		const float screenX = (clip.x * inverseW + 1.0f) * 512.0f;
		const float screenY = (2.0f - (clip.y * inverseW + 1.0f)) * 384.0f;
		::GUI::Window* root = GetRoots()->front();
		LONG width, height;
		root->GetSize(width, height);
		LONG x = LONG(static_cast<LONGLONG>(screenX));
		LONG y = LONG(static_cast<LONGLONG>(screenY)) - height;
		static_cast<Client*>(F3::g_pNetworkInstance->client)->child0xA0->GetScreenRectangle(
			screenLeft, screenTop, screenWidth, screenHeight);
		if (x + width < 0 || x > screenWidth || y + height < 0 || y > screenHeight)
			Close();
		else
		{
			if (x < 0) x = 0;
			if (x + width > screenWidth) x = screenWidth - width;
			if (y < 0) y = 0;
			if (y + height > screenHeight) y = screenHeight - height;
			root->SetAbsolutePosition(x, y);
		}
	}

	int CSMenuInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (!event->type)
		{
			if (window == GetRoots()->front() && fadeStart)
			{
				const DWORD time = GetWorldInstance()->GetTime();
				Entity* entity = GetEntityPtr(actorID);
				if (!(entity && entity->moving != 1 && entity->renderEnabled
					&& entity->IsDead() != 1 && entity->IsVisible()
					&& (entity = GetEntityPtr(targetID)) && entity->moving != 1
					&& entity->renderEnabled && entity->IsDead() != 1 && entity->IsVisible()))
					fadeStart = time;
				if (time >= fadeStart) Fade(time);
			}
		}
		else if (event->type == 6)
		{
			if (!_stricmp(window->GetName(), "CSMenuLabel")) Close();
		}
		return EngineInterface::OnEvent(window, event);
	}
}

namespace F3
{
	BYTE GameEntity::ShowSkills() //467240.
	{
		SkillMenuEntry temporary(0); //Original constructs the unused string-bearing record.
		JE::Script_Small* script = GetScript();
		if (!script || !script->HasPublic("ShowSkills")) return 0;
		cell result; //Original leaves this unwritten before Run, including its failure path.
		if (!script->Run("ShowSkills", &result))
			JE::LogDebugString("Couldn't execute GetShowSkills on the entity %s\r\n",
				scriptName.c_str());
		if (result) return 1; //4672E5 also follows the diagnostic; no failure substitute.
		return 0;
	}

	void GameEntity::GetUsableSkills(JE::Entity*,
		std::list<SkillMenuEntry>& entries) //4687C0; incoming actor is unused here.
	{
		SkillMenuEntry entry(0);
		JE::Script_Small* script = GetScript();
		if (!script || !script->HasPublic("GetUsableSkills")) return;
		cell result, values[26]; //Original array is not initialized before the wrapper.
		if (!script->RunArray("GetUsableSkills", &result, values, 26))
			JE::LogDebugString("Couldn't execute GetUsableSkills on the entity %s\r\n",
				scriptName.c_str());
		//46888F scans the unchanged output storage after either success or failure.
		for (DWORD i = 0; i < 26; i += 2)
		{
			if (values[i])
			{
				entry.SetStringReference(DWORD(values[i])); //Actual ordinary CALL568420.
				entry.selector = DWORD(values[i + 1]);
				entries.push_back(entry);
			}
		}
	}

	GameCSSkillMenuInterface::GameCSSkillMenuInterface(const std::string& name)
		: JE::EngineInterface(name), GameInterface(name), JE::CSMenuInterface(name) {}

	GameCSSkillMenuInterface::~GameCSSkillMenuInterface()
	{
	}

	void GameCSSkillMenuInterface::Populate(DWORD actorID, DWORD targetID)
	{
		JE::Entity* actor = JE::GetEntityPtr(actorID);
		GameEntity* target = dynamic_cast<GameEntity*>(JE::GetEntityPtr(targetID));
		std::string text;
		if (actor && target)
		{
			std::list<SkillMenuEntry> entries;
			target->GetUsableSkills(actor, entries);
			for (std::list<SkillMenuEntry>::iterator i = entries.begin(); i != entries.end(); ++i)
			{
				SkillMenuEntry entry = *i;
				if (LONG(entry.stringReference) > 0)
				{
					GetString(entry.stringReference, text); //Original ignores lookup failure here.
					AddLabel(text, actorID, targetID, entry.selector,
						entry.value24, entry.value28, entry.disabled);
				}
				else AddLabel(entry.text, actorID, targetID, entry.selector,
					entry.value24, entry.value28, entry.disabled);
			}
		}
		if (!GetString(158, text)) text = "<bad cancel string>";
		AddLabel(text, 0, 0, 0, 0, 0, 0);
	}

	int GameCSSkillMenuInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (event->type == 6 && !_stricmp(window->GetName(), "CSMenuLabel"))
		{
			JE::CSMenuLabel* label = dynamic_cast<JE::CSMenuLabel*>(window);
			if (label && !label->disabled)
			{
				QueueClientSkillSelection(label->actorID, label->targetID, label->selector);
				Close();
			}
		}
		return JE::CSMenuInterface::OnEvent(window, event);
	}

	GameSkillMenu::GameSkillMenu(const std::string& name)
		: JE::EngineInterface(name), GameInterface(name), JE::CSMenuInterface(name), skills(name) {}

	GameSkillMenu::~GameSkillMenu()
	{
	}

	void GameSkillMenu::Populate(DWORD actorID, DWORD targetID)
	{
		static DWORD references[] = {154,155,156,157,158,990,991};
		JE::Entity* actor = JE::GetEntityPtr(actorID);
		GameEntity* target = dynamic_cast<GameEntity*>(JE::GetEntityPtr(targetID));
		std::string text;
		if (actor && target)
		{
			std::list<SkillMenuEntry> entries;
			target->GetContextActions(actor, entries);
			for (std::list<SkillMenuEntry>::iterator i = entries.begin(); i != entries.end(); ++i)
			{
				SkillMenuEntry entry = *i;
				if (entry.selector == 3 && !target->ShowSkills()) continue;
				if (!entry.stringReference)
				{
					//Original6F2D68 table, all seven authored string references.
					GetString(references[entry.selector], text);
					AddLabel(text, actorID, targetID, entry.selector,
						entry.value24, entry.value28, entry.disabled);
				}
				else if (LONG(entry.stringReference) > 0)
				{
					//435B72 resolves a positive reference but still passes the literal.
					GetString(entry.stringReference, text);
					AddLabel(entry.text, actorID, targetID, entry.selector,
						entry.value24, entry.value28, entry.disabled);
				}
				else AddLabel(entry.text, actorID, targetID, entry.selector,
					entry.value24, entry.value28, entry.disabled);
			}
		}
		if (!GetString(references[4], text)) text = "<bad cancel string>";
		AddLabel(text, 0, 0, 0, 0, 0, 0);
	}

	int GameSkillMenu::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (event->type == 6 && !_stricmp(window->GetName(), "CSMenuLabel"))
		{
			JE::CSMenuLabel* label = dynamic_cast<JE::CSMenuLabel*>(window);
			JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
			if (label && !label->disabled)
			{
				JE::Entity* actor = JE::GetEntityPtr(label->actorID);
				JE::Entity* target = JE::GetEntityPtr(label->targetID);
				switch (label->selector)
				{
				case 0:
					if (actor && target) client->RequestTalk(label->actorID,label->targetID);
					break;
				case 1:
					if (actor && target) client->RequestLook(label->actorID,label->targetID);
					break;
				case 3:
					skills.Show(label->actorID,label->targetID,event->x,event->y,4000,4000);
					break;
				case 5:
				case 6:
				{
					GameDoor* door = dynamic_cast<GameDoor*>(target);
					if (door)
					{
						if ((label->selector == 5 && !door->open)
							|| (label->selector == 6 && door->open == 1))
						{
							GameActionOperateDoor* action = new(std::nothrow) GameActionOperateDoor;
							action->SetDoor(door);
							actor->QueueAction(action);
						}
					}
					else if (label->selector == 5)
					{
						GameContainer* container = dynamic_cast<GameContainer*>(target);
						GameCreature* creature = dynamic_cast<GameCreature*>(target);
						if (container || creature)
						{
							GameActionOperateContainer* action = new(std::nothrow) GameActionOperateContainer;
							action->SetTarget(target);
							actor->QueueAction(action);
						}
					}
					break;
				}
				} //Original cases2/4/default perform no action.
				Close();
			}
		}
		//43577E passes main+D54:5687B0 expects its secondary interface
		//receiver and subtracts D54 before Close. This is the main base handler,
		//even though that adjusted address aliases the embedded skills primary.
		return JE::CSMenuInterface::OnEvent(window, event);
	}

	void GameSkillMenu::Open(DWORD actorID, DWORD targetID, LONG x, LONG y)
	{
		skills.Show(actorID, targetID, x, y, 4000, 4000);
	}


}
