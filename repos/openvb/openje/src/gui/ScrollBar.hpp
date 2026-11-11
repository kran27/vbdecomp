#pragma once

#include <windows.h>

namespace GUI
{
    class Window;
    struct Event;

    // Actual GUI::ScrollBar670064: independent ordinary owner, borrowed widgets.
    // Original104-byte state; normal fields, no legacy object/storage overlay.
    class ScrollBar
    {
    public:
        ScrollBar(); //4B8720.
        virtual ~ScrollBar(); //4B8780; compiler supplies4B8A80 deleting glue.
        void BindArrows(Window* decrease, Window* increase); //4B8790.
        void BindThumb(Window* value); //4B87B0; captures its dimensions when nonnull.
        void BindTrack(Window* value); //4B87E0; captures actual absolute rectangle.
        void SetPageStep(LONG value); //4B87D0.
        void SetFlags(DWORD value); //Shared59C2C0, only bit0 selects horizontal.
        void SetMaximum(LONG value); //4B8AA0; does not clamp current value.
        void SetPageSize(LONG value); //4B8AC0.
        void SetStep(LONG value); //Shared568420.
        void SetValue(LONG value, BYTE refresh); //4B8AE0; raw store, optional geometry.
        LONG GetValue(); //Shared544A10.
        BYTE Process(Window* window, Event* event); //4B8B90.

        Window* decreaseButton; //+04, borrowed.
        Window* increaseButton; //+08, borrowed.
        Window* thumb; //+0C, borrowed.
        Window* track; //+10, borrowed.
        DWORD flags; //+14.
        LONG maximum, pageSize, step, pageStep, value; //+18..28.
        LONG thumbWidth, thumbHeight, pressedDirection; //+2C/30/34.
        BYTE dragging; //+38; original padding39..3B remains unwritten.
        LONG dragOffsetX, dragOffsetY; //+3C/40.
        LONG trackX, trackY, trackWidth, trackHeight; //+44..50.
        DWORD holdStartedAt, repeatedAt, holdDelay, repeatDelay; //+54..60.
        BYTE disabled; //+64; original tail padding unwritten.

    private:
        void UpdateThumb(); //4B8840.
        BOOL Decrease(LONG amount); //4B88F0.
        BOOL Increase(LONG amount); //4B8930.
        LONG DirectionAt(LONG x, LONG y); //4B8980.
        LONG ValueAt(LONG x, LONG y); //4B8A00.
        BYTE DragTo(LONG x, LONG y); //4B8B00.
    };
}
