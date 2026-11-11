#pragma once

#include <windows.h>
#include "amx.h"
#include <stdio.h>
#include <list>
#include <map>

namespace JE
{
	namespace Common { class Chunk; }
	namespace Resources { struct ResourceMemoryStream; }
	// Gameplay effect owner (55B230), distinct from graphics VFX_EffectInstance.
	class Entity;
	class Effect
	{
	public:
		explicit Effect(DWORD type);
		virtual BYTE IsComplete() const; //45C090, original slot0.
		virtual DWORD GetRole() const = 0; //Original slot4, pure in the engine base.
		virtual void CopyFrom(const Effect& source); //55AE00 forwards to assignment, original slot8.
		virtual ~Effect(); //55B130, original slotC; children delete without deactivation.
		virtual BYTE Write(FILE* stream); //55A2A0.
		virtual BYTE ReadFromResource(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //5A0BB0, original+14.
		virtual BYTE Read(FILE* stream); //55A300, consumes/discards serialized type.
		virtual BYTE CanRefresh() const; //408D30, original slot1C.
		virtual BYTE OnInitialize() = 0; //Original slot20.
		virtual void OnBegin() = 0; //Original slot24.
		virtual BYTE OnActivate() = 0; //Original slot28, caller tests AL.
		virtual void OnDeactivate() = 0; //Original slot2C.
		virtual BYTE OnUpdate() = 0; //Original slot30, governs child traversal.
		virtual BYTE UnresolvedSlot34() = 0; //Original pure+34; meaning and source spelling unknown.

		Effect& operator=(const Effect& source); //55AC10; preserves the destination identity.
		BYTE RunScript(Entity* entity, const char* name, cell* result, int count, cell* arguments); //55A220; real Entity forwarding, incoming Effect unused.
		DWORD GetID(); //55A240; allocate through the current World only while effectID==0.
		BYTE ReadResource(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream); //55A280; normalizes virtual reader AL.
		void AddChild(Effect* child); //55ADC0; ignores NULL, otherwise owns the appended child.
		void SetCreator(DWORD id); //55A360; each setter also visits existing children.
		void SetOwner(DWORD id); //55A3A0.
		void SetTarget(DWORD role, DWORD id); //55A3E0; original caller requires role<4.
		void SetEnabled(BYTE value); //55A420.
		void InitializeTree(); //55A460; failed initialization publishes completed=1.
		void BeginTree(); //55A4B0.
		void ActivateTree(); //55AE10.
		void DeactivateTree(); //55A4F0.
		void RefreshTree(); //55AF80.
		void UpdateTree(); //55AFB0.
		void Destroy(); //55A570: deactivate, then virtual delete.
		void RemoveChild(Effect* child); //55A590; node-only removal, no delete.

		DWORD effectID; //+4, ctor0; lazy allocation55A240.
		BYTE initialized, started, activated, completed; //+8..B, all ctor0.
		DWORD type, creatorID, ownerID; //+C,+10,+14.
		std::list<Effect*> children; //+18, owned children; live traversal.
		DWORD value24; //+24, ctor0; nonzero prevents direct ActivateTree.
		DWORD targets[4]; //+28..34, ctor0.
		BYTE enabled; //+38, ctor1.
		std::map<DWORD,DWORD> values; //+3C, actual24-byte tree nodes; normal STL.
	private:
		Effect(const Effect&);
	};
}
