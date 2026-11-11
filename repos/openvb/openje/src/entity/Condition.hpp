#pragma once

#include "Effect.hpp"
#include <string>

namespace JE
{
	//Original RTTI Condition, gameplay effect type0; not a graphics VFX owner.
	class Condition : public Effect
	{
	public:
		explicit Condition(DWORD type); //5A8D30.
		virtual ~Condition(); //5A8D80.
		virtual BYTE IsComplete() const; //5A8C40, marks a childless condition complete.
		virtual DWORD GetRole() const; //5A8D70.
		Condition& operator=(const Condition& source); //5A8ED0.
		virtual void CopyFrom(const Effect& source); //5A8F10 forwarding entry.
		virtual BYTE Write(FILE* stream); //5A8C00, name only, not Effect's FILE prefix.
		virtual BYTE Read(FILE* stream); //5A8C20.
		virtual BYTE ReadFromResource(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //5A8F20.
		virtual BYTE OnInitialize(); //408D30, true.
		virtual void OnBegin(); //404B70, no-op.
		virtual BYTE OnActivate(); //5A8DC0.
		virtual void OnDeactivate(); //404B70, no-op.
		virtual BYTE OnUpdate(); //5A8E00.
		virtual BYTE UnresolvedSlot34(); //Original+34, shared408D30 returns AL1, RET0.
		BYTE RunScript(); //5A8C50, actual shared conditions.amx owner and real Small VM.
		std::string functionName;
		DWORD targetRole; //Original+64, constructor-unwritten; resource reader supplies it.
		DWORD role; //Original+68, constructor0; optional resource version2 field.
		BYTE cachedResult, evaluated; //Original+6C/+6D, constructor0.
	};
}
