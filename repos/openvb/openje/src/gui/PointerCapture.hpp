#pragma once

namespace JE
{
	namespace GUI
	{
		// Source-level interface for original Window virtual slot +0x118.
		// Recompiled classes use normal C++ dispatch, not a fabricated old table.
		class CaptureTarget
		{
		public:
			virtual void OnCaptureChanged(CaptureTarget* pNext) = 0;
		protected:
			~CaptureTarget() {}
		};

		// The currently reconstructed member of canonical Manager 0x707E48 (+0xB8).
		// Full manager construction and window classes remain separate work. When
		// porting that owner, move this state into it rather than adding a second owner.
		struct PointerCaptureState
		{
			CaptureTarget* pTarget;
		};

		extern PointerCaptureState g_PointerCapture;
		unsigned char SetCapturedWindow(PointerCaptureState& state, CaptureTarget* pNext);
	}
}
