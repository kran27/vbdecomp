#include "startup/NetworkComponent.hpp"
#include "Condition.hpp"
#include "Chunk.hpp"
#include "Entity.hpp"
#include "EntityEnvironment.hpp"
#include "World.hpp"
#include "scripting/ScriptSmall.hpp"
#include "resources/ResourceSystem.hpp"
#include "model/CommonException.hpp"
#include <vector>
#include <stdexcept>

namespace
{
	BYTE conditionScriptFailed; //Original zero-initialized70C734, permanent failed-load latch.
	JE::Script_Small* conditionScript; //Original70C738, shared across Conditions/World instances.
	//The original has no other xrefs or normal cleanup producer for this pointer.
	//Keep that lifetime; do not delete the shared VM in each Condition destructor.
}

namespace JE
{
	Condition::Condition(DWORD effectType) : Effect(effectType)
	{
		cachedResult=0;
		evaluated=0;
		role=0;
		//targetRole is constructor-unwritten until the resource/copy reader supplies it.
	}
	Condition::~Condition() {}
	BYTE Condition::IsComplete() const
	{
		if (children.empty()) const_cast<Condition*>(this)->completed=1;
		return completed;
	}
	DWORD Condition::GetRole() const { return role; }
	Condition& Condition::operator=(const Condition& source)
	{
		Effect::operator=(source);
		functionName=source.functionName;
		targetRole=source.targetRole;
		role=source.role;
		cachedResult=source.cachedResult;
		evaluated=source.evaluated;
		return *this;
	}
	BYTE Condition::Write(FILE* stream) //5A8C00 ->49C6C0, no Effect FILE prefix.
	{
		return Common::WriteString(functionName, stream) != 0;
	}
	BYTE Condition::Read(FILE* stream) //5A8C20 ->49C800.
	{
		return Common::ReadString(functionName, stream) != 0;
	}
	BYTE Condition::ReadFromResource(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream)
	{
		if (!Effect::ReadFromResource(chunk,stream)) return 0;
		char name[260];
		if (!Resources::g_pResourceSystem->ReadMemory(name,sizeof(name),stream)) return 0;
		functionName.assign(name); //Original fixed field requires a terminated resource string.
		if (!Resources::g_pResourceSystem->ReadMemory(&targetRole,4,stream)) return 0;
		return chunk.version<2 || Resources::g_pResourceSystem->ReadMemory(&role,4,stream);
	}
	BYTE Condition::OnInitialize() { return 1; }
	BYTE Condition::UnresolvedSlot34() { return 1; }
	void Condition::OnBegin() {}
	void Condition::OnDeactivate() {}
	BYTE Condition::RunScript() //5A8C50.
	{
		if (!(F3::g_pNetworkInstance->GetMode()&1))
			throw ::Common::Exception("Condition::RunScript() method may only be called by the server.");
		if (conditionScriptFailed) return 0;
		Script_Small* active = conditionScript;
		if (!active)
		{
			IWorldServer* server = GetWorldServerInstance();
			Script_Small* created = server->NewScript(0);
			conditionScript = created;
			BYTE loaded = created->Load("conditions.amx",1);
			active = conditionScript;
			if (!loaded)
			{
				conditionScriptFailed = 1;
				delete active;
				conditionScript = NULL;
				return 0;
			}
		}
		//The original indexes the four target roles unchecked; no invented role clamp.
		cell argument=cell(targets[targetRole]);
		cell result;
		return active->Run(functionName.c_str(),&result,1,&argument) && result!=0;
	}

	BYTE Condition::OnActivate() //5A8DC0.
	{
		if (enabled && evaluated) return cachedResult;
		BYTE result=RunScript();
		cachedResult=result;
		evaluated=1;
		if (!result && enabled) completed=1;
		return result;
	}
	BYTE Condition::OnUpdate() //5A8E00.
	{
		if (enabled) return cachedResult;
		BYTE result=RunScript();
		if (result==cachedResult) return result;
		if (cachedResult)
		{
			for (std::list<Effect*>::iterator i=children.begin();i!=children.end();++i)
			{
				(*i)->DeactivateTree();
				Entity* owner=GetEntityPtr(ownerID);
				owner->associationsDirty=1;
			}
			activated=0;
		}
		else
		{
			for (std::list<Effect*>::iterator i=children.begin();i!=children.end();++i) (*i)->ActivateTree();
			activated=1;
		}
		cachedResult=result;
		return result;
	}
}
