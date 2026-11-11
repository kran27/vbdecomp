#include "Combat.hpp"
#include "Entity.hpp"
#include "EntityEnvironment.hpp"
#include "World.hpp"
#include "model/CommonException.hpp"
#include "startup/ConfigurationObject.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace JE
{
	Combat::Combat() : mode(0), active(0), cursor(), timeOffset(0), stopRequested(0), showOutlines(1) {}
	Combat::~Combat()
	{
		SaveSettings();
		Cleanup();
	}
	void Combat::Initialize()
	{
		mode = 0;
		showOutlines = 1;
		Configuration::Settings& settings = *F3::GetSettingsObject();
		Configuration::Settings::const_iterator section = settings.find("Combat");
		if (section == settings.end()) return;
		const Configuration::Values& values=section->second;
		Configuration::Values::const_iterator value = values.find("mode");
		if (value != values.end()) mode = std::atol(value->second.c_str());
		value = values.find("show outlines");
		if (value != values.end()) showOutlines = std::atol(value->second.c_str()) != 0;
	}
	void Combat::SaveSettings()
	{
		Configuration::Settings& groups = *F3::GetSettingsObject();
		Configuration::Values& settings = groups["Combat"];
		char buffer[36];
		settings["mode"] = ::_itoa(static_cast<int>(mode), buffer, 10);
	}
	void Combat::OnActorsChanged(DWORD) {} // Original5A9320, one argument.
	void Combat::SetMode(DWORD value)
	{
		if (value==1)
		{
			for (std::list<Entity*>::iterator i=g_createdEntities.begin();i!=g_createdEntities.end();++i)
				if ((*i)->flag296) (*i)->FreezeAnimation();
		}
		else
		{
			for (std::list<Entity*>::iterator i=g_createdEntities.begin();i!=g_createdEntities.end();++i)
				if ((*i)->flag296) (*i)->UnfreezeAnimation();
		}
		mode = value;
		stopRequested = 0;
	}
	void Combat::AddEntity(Entity* entity)
	{
		if (std::find(actors.begin(), actors.end(), entity) != actors.end()) return;
		actors.push_front(entity);
		entity->ChangeState(5);
		OnActorsChanged(GetWorldServerInstance()->GetServerTime());
	}
	void Combat::RemoveEntity(Entity* entity)
	{
		std::list<Entity*>::iterator i = std::find(actors.begin(), actors.end(), entity);
		if (i == actors.end()) return;
		Entity* removed = *i;
		actors.erase(i);
		if (removed->state == 5) removed->ChangeState(4);
		OnActorsChanged(GetWorldServerInstance()->GetServerTime());
	}
	void Combat::ClearEntities()
	{
		std::list<Entity*>::iterator i = actors.begin();
		while (i != actors.end())
		{
			Entity* entity = *i;
			i = actors.erase(i);
			entity->ChangeState(4);
		}
	}
	void Combat::Cleanup()
	{
		ClearEntities();
		mode = 0;
		active = 0;
		timeOffset = 0;
		stopRequested = 0;
	}
	void Combat::Update(DWORD time)
	{
		if (!active)
		{
			active = 1;
			std::list<Entity*>::iterator i = actors.begin();
			while (i != actors.end())
			{
				Entity* entity = *i;
				if (entity->state != 5) i = actors.erase(i);
				else
				{
					entity->UpdateClient(time);
					++i;
				}
			}
			cursor = actors.begin();
		}
		if (actors.empty())
		{
			active = 0;
			return;
		}
		if (!mode)
		{
			while (cursor != actors.end())
			{
				(*cursor)->ServerUpdate(time);
				++cursor;
			}
			active = 0;
			return;
		}
		Entity* entity = *cursor;
		DWORD offset = 0;
		if (entity->actionBlocked)
		{
			entity->value29C = 0;
			++cursor;
			if (stopRequested)
			{
				SetMode(0);
				return; // Original leaves active unchanged on this early return.
			}
			if (cursor == actors.end())
			{
				active = 0;
				return;
			}
			entity = *cursor;
		}
		else
		{
			offset = 1000 * GetWorldServerInstance()->GetSecondsPerRound();
			offset += timeOffset;
		}
		timeOffset = offset;
		if (!entity->visibilityOwnerFlag || !entity->actions.empty())
		{
			entity->value29C = timeOffset / 1000;
			entity->ServerUpdate(time + timeOffset);
		}
	}
}
