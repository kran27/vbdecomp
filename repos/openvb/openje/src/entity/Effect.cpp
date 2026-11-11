#include "startup/NetworkComponent.hpp"
#include "Effect.hpp"
#include "Condition.hpp"
#include "Entity.hpp"
#include "EntityEnvironment.hpp"
#include "World.hpp"
#include "model/CommonException.hpp"
#include <stdexcept>

namespace JE
{
	Effect::Effect(DWORD effectType)
	{
		effectID=0;
		initialized=0; started=0; activated=0; completed=0;
		creatorID=0; ownerID=0;
		type=effectType;
		for (DWORD i=0;i<4;++i) targets[i]=0;
		enabled=1;
		value24=0;
	}
	Effect::~Effect()
	{
		while (!children.empty())
		{
			Effect* child=children.front();
			children.pop_front(); //55B18E unlinks before the virtual destructor.
			delete child;
		}
	}
	BYTE Effect::IsComplete() const { return completed; }
	BYTE Effect::CanRefresh() const { return 1; }
	BYTE Effect::RunScript(Entity* entity, const char* name, cell* result, int count, cell* arguments)
	{
		return entity->RunScript(name, result, count, arguments);
	}
	DWORD Effect::GetID()
	{
		if (!effectID)
		{
			IWorld* world=g_World;
			effectID=world->AllocateEffectID();
		}
		return effectID;
	}
	BYTE Effect::ReadFromResource(Common::Chunk&, Resources::ResourceMemoryStream*) { return 1; }
	BYTE Effect::ReadResource(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream)
	{
		return ReadFromResource(chunk, stream) != 0;
	}
	void Effect::AddChild(Effect* child)
	{
		if (child) children.push_back(child);
	}
	BYTE Effect::Write(FILE* stream)
	{
		return fwrite(&type,4,1,stream)==1 && fwrite(&creatorID,4,1,stream)==1
			&& fwrite(&ownerID,4,1,stream)==1;
	}
	BYTE Effect::Read(FILE* stream)
	{
		DWORD ignoredType; //55A309 reads into the stack parameter, not this->type.
		return fread(&ignoredType,4,1,stream)==1 && fread(&creatorID,4,1,stream)==1
			&& fread(&ownerID,4,1,stream)==1;
	}
	void Condition::CopyFrom(const Effect& input)
	{
		*this=static_cast<const Condition&>(input);
	}
	Effect& Effect::operator=(const Effect& source)
	{
		initialized=source.initialized; started=source.started;
		activated=source.activated; completed=source.completed;
		type=source.type; creatorID=source.creatorID; ownerID=source.ownerID;
		value24=0;
		for (DWORD i=0;i<4;++i) targets[i]=source.targets[i];
		for (std::map<DWORD,DWORD>::const_iterator i=source.values.begin();i!=source.values.end();++i)
		{
			DWORD value=i->second;
			values[i->first]=value; //Original snapshots and merges without clearing destination keys.
		}
		enabled=source.enabled;
		for (std::list<Effect*>::const_iterator i=source.children.begin();i!=source.children.end();++i)
		{
			Effect* sourceChild=*i;
			if (!(F3::g_pNetworkInstance->GetMode()&1))
				throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
			IWorldServer* server=g_World;
			Effect* child=server->NewEffect(sourceChild->type);
			child->CopyFrom(*sourceChild);
			children.push_back(child);
		}
		return *this;
	}
	void Effect::CopyFrom(const Effect& source)
	{
		*this=source;
	}
	void Effect::SetCreator(DWORD id)
	{
		creatorID=id;
		for (std::list<Effect*>::iterator i=children.begin();i!=children.end();++i) (*i)->SetCreator(id);
	}
	void Effect::SetOwner(DWORD id)
	{
		ownerID=id;
		for (std::list<Effect*>::iterator i=children.begin();i!=children.end();++i) (*i)->SetOwner(id);
	}
	void Effect::SetTarget(DWORD role,DWORD id)
	{
		targets[role]=id;
		for (std::list<Effect*>::iterator i=children.begin();i!=children.end();++i) (*i)->SetTarget(role,id);
	}
	void Effect::SetEnabled(BYTE value)
	{
		enabled=value;
		for (std::list<Effect*>::iterator i=children.begin();i!=children.end();++i) (*i)->SetEnabled(value);
	}
	void Effect::InitializeTree()
	{
		if (initialized) return;
		if (OnInitialize())
		{
			initialized=1;
			for (std::list<Effect*>::iterator i=children.begin();i!=children.end();++i) (*i)->InitializeTree();
		}
		else completed=1;
	}
	void Effect::BeginTree()
	{
		if (started) return;
		OnBegin();
		started=1;
		for (std::list<Effect*>::iterator i=children.begin();i!=children.end();++i) (*i)->BeginTree();
	}
	void Effect::RemoveChild(Effect* child) { if (child) children.remove(child); }
	void Effect::ActivateTree()
	{
		std::list<Effect*> retired;
		if (activated || value24) return;
		if (OnActivate())
		{
			activated=1;
			for (std::list<Effect*>::iterator i=children.begin();i!=children.end();++i)
			{
				Effect* child=*i;
				child->ActivateTree();
				if (child->IsComplete()) retired.push_back(child);
			}
		}
		for (std::list<Effect*>::iterator i=retired.begin();i!=retired.end();++i)
		{
			Effect* child=*i;
			RemoveChild(child);
			child->Destroy();
		}
	}
	void Effect::DeactivateTree()
	{
		if (!activated) return;
		activated=0; //55A4FD publishes before the virtual callback.
		OnDeactivate();
		for (std::list<Effect*>::iterator i=children.begin();i!=children.end();++i) (*i)->DeactivateTree();
	}
	void Effect::RefreshTree()
	{
		if (activated && CanRefresh())
		{
			activated=0;
			ActivateTree();
		}
	}
	void Effect::UpdateTree()
	{
		std::list<Effect*> retired;
		if (completed) return;
		if (OnUpdate())
		{
			for (std::list<Effect*>::iterator i=children.begin();i!=children.end();++i)
			{
				Effect* child=*i;
				child->UpdateTree();
				if (child->IsComplete())
				{
					retired.push_back(child);
					GetEntityPtr(ownerID)->associationsDirty=1;
				}
			}
		}
		for (std::list<Effect*>::iterator i=retired.begin();i!=retired.end();++i)
		{
			Effect* child=*i;
			RemoveChild(child);
			child->Destroy();
		}
	}
	void Effect::Destroy()
	{
		DeactivateTree();
		delete this;
	}
}
