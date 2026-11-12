#pragma once

#include <d3dx8.h>
#include <string>

namespace JE
{
	class SoundPresetInfo;
	class SoundGroup;
	struct SoundFileEntry;

	// Actual outer52E2E0 owner; distinct from its per-file CSound4C8100 children.
	class VfxSoundInstance
	{
	public:
		VfxSoundInstance(); //52E2E0, registers after its actual scalar initialization.
		~VfxSoundInstance(); //52D100, unregister before preset/entry destruction.
		int Load(const char* name,DWORD flags=0,const char* overrideName=NULL); //52D820.
		D3DXVECTOR3* GetPosition(D3DXVECTOR3* output) const; //52C370, typed native three-word output boundary.
		void SetPosition(D3DXVECTOR3 value); //52C350, actual by-value vector/RET12 boundary.
		DWORD IsPlaying(); //52CC40, actual52C5D0/52C610 software flags.
		DWORD GetStartedAt() const; //544A00, shared +24 getter; no unique body credit.
		int Stop(DWORD value); //52C580, reset sequencing then per-file stop child.
		int Priority(); //52C710, maximum current per-file volume, not admission.
		int SetPriority(int value); //52C6C0, per-file volume requests.
		int Play(DWORD flags,DWORD fadeIn); //52CCA0, authored simultaneous/ordered/random selection.
		void Update(); //52CA70, sequence advancement and interaction-volume refresh.
		int Retire(); //52C530, immediate per-file stop and reset sequencing.
		int HasActiveSound(); //52C5D0, includes a live sequence.
		int HasPendingSound(); //52C610, pending is false when any voice is active.
		int CanAutoDelete(); //52C660, fire-and-forget/nonloop/completed voice condition.

		SoundPresetInfo* preset; //+00, owned.
		std::string resourceName; //+04, actual requested SPF/WAV name.
		SoundGroup* group; //+20, borrowed actual master-group record.
		DWORD startedAt; //+24.
		LONG sequenceIndex; //+28, initial-1.
		DWORD loadFlags; //+2C, Load's second argument.
		DWORD flags; //+30, raw playback flags, distinct from loadFlags.
		BYTE spatial; //+34.
		D3DXVECTOR3 position; //+38/+3C/+40.
	private:
		BOOL IsSoundPreset(const char* name); //52C3C0, native Load supplies this/RET4.
		int Random(int minimum,int maximum); //52C390, native callers pass this; callee does not consume it.
		int PlayEntry(SoundFileEntry* entry,DWORD flags,DWORD fadeIn); //52C960.
		VfxSoundInstance(const VfxSoundInstance&);
		VfxSoundInstance& operator=(const VfxSoundInstance&);
	};
}
