#include "PointerCapture.hpp"

namespace JE
{
	namespace GUI
	{
		// Manager ctor 0x4B5640 explicitly clears this member at 0x4B5770.
		// This is member-state ownership, not a reconstructed complete Manager ctor.
		PointerCaptureState g_PointerCapture = { 0 };

		// Original VA 0x4B2AD0; non-owning pointers, callback before final store.
		unsigned char SetCapturedWindow(PointerCaptureState& state, CaptureTarget* pNext)
		{
			CaptureTarget* pPrevious = state.pTarget;
			if (pPrevious == pNext)
			{
				return 0;
			}
			if (pPrevious)
			{
				pPrevious->OnCaptureChanged(pNext);
			}
			// A returning callback's changes are overwritten; a throw skips this.
			state.pTarget = pNext;
			return 1;
		}
	}
}
