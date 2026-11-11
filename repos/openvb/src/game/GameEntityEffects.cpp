#include "startup/NetworkComponent.hpp"
#include "GameEntity.hpp"
#include "entity/Effect.hpp"
#include "entity/World.hpp"
#include "resources/ResourceSystem.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "model/CommonException.hpp"

namespace F3
{
	BYTE GameEntity::ReadEffectRecord(JE::Common::Chunk& eventChunk,
		JE::Resources::ResourceMemoryStream* stream, JE::Effect* parent, DWORD event)
	{
		if (!(g_pNetworkInstance->GetMode() & 1)) return 1;

		DWORD childCount = 0;
		//468537/468597 address distinct payload words in the same stack frame.
		DWORD ignoredTypeHeader;
		DWORD ignoredConditionHeader;
		DWORD type;
		JE::Common::Chunk record;
		if (!record.Read(stream)) goto failed;
		JE::Effect* effect;
		if (record.type != 'DCFE')
		{
			if (record.type != 'TDFE')
			{
				JE::LogDebugString("Unknown effect chunk ID.");
				goto failed;
			}
			if (!JE::Resources::g_pResourceSystem->ReadMemory(&childCount, 4, stream)
				|| !JE::Resources::g_pResourceSystem->ReadMemory(&ignoredTypeHeader, 4, stream)
				|| !JE::Resources::g_pResourceSystem->ReadMemory(&type, 4, stream)) goto failed;
			effect = JE::GetWorldServerInstance()->NewEffect(type);
			effect->ReadResource(record, stream);
		}
		else
		{
			if (!JE::Resources::g_pResourceSystem->ReadMemory(&childCount, 4, stream)
				|| !JE::Resources::g_pResourceSystem->ReadMemory(&ignoredConditionHeader, 4, stream)) goto failed;
			effect = JE::GetWorldServerInstance()->NewEffect(0);
			//4685C4/4685CA: payload reads join after each branch's factory call.
			effect->ReadResource(record, stream); //The result is deliberately ignored.
		}

		if (parent) parent->AddChild(effect);
		else effects[event].push_back(effect);

		//Publish before recursion. Failed children leave the partial owner graph.
		for (DWORD child = 0; child < childCount; ++child)
			if (!ReadEffectRecord(eventChunk, stream, effect, event)) goto failed;
		record.SkipToEnd();
		return 1;
	failed: //468516: all payload/recursive failures share this return.
		return 0;
	}

	void GameEntity::ApplyOnCreateEffects()
	{
		std::list<JE::Effect*>* templates=GetEffects(3);
		if (templates->empty()) return;
		for (std::list<JE::Effect*>::iterator entry=templates->begin();
			entry!=templates->end(); ++entry)
		{
			JE::Effect* source = *entry;
			if (!(g_pNetworkInstance->GetMode()&1))
				throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
			JE::IWorldServer* server=JE::g_World;
			JE::Effect* effect=server->NewEffect(source->type);
			effect->CopyFrom(*source);
			effect->SetCreator(entityID);
			effect->SetTarget(1, entityID);
			effect->SetTarget(2, entityID);
			effect->SetTarget(0, 0);
			effect->SetTarget(3, 0);
			switch (effect->GetRole())
			{
			case 1:
			case 2:
				AddEffect(effect);
				break;
			case 0:
			case 3:
				JE::LogDebugString("ApplyOnCreateEffects() - All OnCreate effects must target Myself (or Subject, which is considered Myself in this case).\n");
				break; //Original leaves the rejected clone allocated.
			default:
				JE::FatalError("ApplyOnCreateEffects() - Unknown target type.");
			}
		}
	}
}
