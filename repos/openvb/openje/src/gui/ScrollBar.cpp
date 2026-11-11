#include "gui/ScrollBar.hpp"
#include "gui/Window.hpp"
#include "gui/Button.hpp"
#include "gui/Event.hpp"
#include "gui/Manager.hpp"
#include <mmsystem.h>

namespace GUI
{
    namespace
    {
        // Preserve original32-bit arithmetic before its signed comparisons/divide.
        LONG Add(LONG first, LONG second) { return LONG(DWORD(first) + DWORD(second)); }
        LONG Subtract(LONG first, LONG second) { return LONG(DWORD(first) - DWORD(second)); }
        LONG Multiply(LONG first, LONG second) { return LONG(DWORD(first) * DWORD(second)); }

        DWORD ButtonState(Window* window)
        {
            //Native type0 directly calls the bound Button slot, including NULL failure.
            return static_cast<Button*>(window)->GetState();
        }
    }

    ScrollBar::ScrollBar()
        : decreaseButton(NULL), increaseButton(NULL), thumb(NULL), track(NULL),
        flags(0), maximum(0), pageSize(0), step(0), pageStep(0), value(0),
        thumbWidth(0), thumbHeight(0), pressedDirection(0), dragging(0),
        dragOffsetX(0), dragOffsetY(0), trackX(0), trackY(0), trackWidth(0),
        trackHeight(0), holdStartedAt(0), repeatedAt(0), holdDelay(400),
        repeatDelay(35), disabled(0)
    {
    }

    ScrollBar::~ScrollBar() {} //Original destroys no borrowed widget.

    void ScrollBar::BindArrows(Window* decrease, Window* increase)
    {
        decreaseButton = decrease;
        increaseButton = increase;
    }
    void ScrollBar::BindThumb(Window* input)
    {
        thumb = input;
        if (input) input->GetSize(thumbWidth, thumbHeight);
    }
    void ScrollBar::BindTrack(Window* input)
    {
        track = input;
        if (input)
        {
            LONG x, y, width, height;
            input->GetAbsolutePosition(x, y);
            track->GetSize(width, height);
            trackX = x; trackY = y; trackWidth = width; trackHeight = height;
        }
    }
    void ScrollBar::SetPageStep(LONG input) { pageStep = input; }
    void ScrollBar::SetFlags(DWORD input) { flags = input; }
    void ScrollBar::SetMaximum(LONG input)
    {
        if (maximum != input) { maximum = input; UpdateThumb(); }
    }
    void ScrollBar::SetPageSize(LONG input)
    {
        if (pageSize != input) { pageSize = input; UpdateThumb(); }
    }
    void ScrollBar::SetStep(LONG input) { step = input; }
    void ScrollBar::SetValue(LONG input, BYTE refresh)
    {
        value = input;
        if (refresh) UpdateThumb();
    }
    LONG ScrollBar::GetValue() { return value; }

    void ScrollBar::UpdateThumb()
    {
        Window* capturedThumb = thumb;
        if (!capturedThumb) return;
        const LONG range = Subtract(maximum, pageSize);
        LONG x, y;
        if (!(flags & 1)) // Original vertical branch precedes the horizontal branch.
        {
            x = trackX;
            if (range <= 0) y = 0;
            else y = Multiply(value, Subtract(trackHeight, thumbHeight)) / range;
            LONG bound = trackY;
            y = Add(bound, y);
            if (y < bound) y = bound;
            bound = Add(bound, Subtract(trackHeight, thumbHeight));
            if (y > bound) y = bound;
        }
        else
        {
            y = trackY;
            if (range <= 0) x = 0;
            else x = Multiply(value, Subtract(trackWidth, thumbWidth)) / range;
            LONG bound = trackX;
            x = Add(bound, x);
            if (x < bound) x = bound;
            bound = Add(bound, Subtract(trackWidth, thumbWidth));
            if (x > bound) x = bound;
        }
        capturedThumb->SetAbsolutePosition(x, y);
    }

    BOOL ScrollBar::Decrease(LONG amount)
    {
        const LONG previous = value;
        value = Subtract(previous, amount);
        if (value < 0) value = 0;
        UpdateThumb();
        return previous != value;
    }
    BOOL ScrollBar::Increase(LONG amount)
    {
        const LONG previous = value;
        value = Add(previous, amount);
        const LONG last = Subtract(maximum, pageSize);
        if (value > last) value = last;
        if (value < 0) value = 0;
        UpdateThumb();
        return previous != value;
    }
    LONG ScrollBar::DirectionAt(LONG x, LONG y)
    {
        LONG thumbX, thumbY;
        thumb->GetAbsolutePosition(thumbX, thumbY);
        if (!(flags & 1))
        {
            if (y < thumbY) return 1;
            if (y >= Add(thumbY, thumbHeight)) return -1;
        }
        else
        {
            if (x < thumbX) return 1;
            if (x >= Add(thumbX, thumbWidth)) return -1;
        }
        return 0;
    }
    LONG ScrollBar::ValueAt(LONG x, LONG y)
    {
        const LONG capturedMaximum = maximum;
        const LONG range = Subtract(capturedMaximum, pageSize);
        if (range <= 0) return 0;
        LONG displacement, extent, thumbExtent;
        if (!(flags & 1)) // Original vertical branch precedes the horizontal branch.
        {
            displacement = Subtract(y, trackY);
            if (capturedMaximum > 1)
                displacement = Add(displacement, trackHeight / (capturedMaximum - 1) / 2);
            extent = trackHeight; thumbExtent = thumbHeight;
        }
        else
        {
            displacement = Subtract(x, trackX);
            if (capturedMaximum > 1)
                displacement = Add(displacement, trackWidth / (capturedMaximum - 1) / 2);
            extent = trackWidth; thumbExtent = thumbWidth;
        }
        extent = Subtract(extent, thumbExtent);
        if (extent > 0) return Multiply(displacement, range) / extent;
        return 0;
    }
    BYTE ScrollBar::DragTo(LONG x, LONG y)
    {
        if (maximum <= pageSize) return 0;
        LONG nextX = LONG(DWORD(x) - DWORD(dragOffsetX));
        LONG nextY = LONG(DWORD(y) - DWORD(dragOffsetY));
        if (!(flags & 1))
        {
            if (nextY < trackY) nextY = trackY;
            const LONG last = LONG(DWORD(trackY) + DWORD(trackHeight) - DWORD(thumbHeight));
            if (nextY > last) nextY = last;
            nextX = trackX;
        }
        else
        {
            if (nextX < trackX) nextX = trackX;
            const LONG last = LONG(DWORD(trackX) + DWORD(trackWidth) - DWORD(thumbWidth));
            if (nextX > last) nextX = last;
            nextY = trackY;
        }
        thumb->SetAbsolutePosition(nextX, nextY);
        const LONG nextValue = ValueAt(nextX, nextY);
        if (value == nextValue) return 0;
        SetValue(nextValue, 0);
        return 1;
    }

    BYTE ScrollBar::Process(Window* window, Event* event)
    {
        if (disabled) return 0;
        const int eventType = event->type;
        if (eventType != 4 && eventType != 7)
        {
            if (eventType != 0) return 0;
            if (window == decreaseButton)
            {
                if (ButtonState(window) == 2)
                {
                    const DWORD now = timeGetTime();
                    if (holdStartedAt + holdDelay < now && repeatedAt + repeatDelay < now)
                    { repeatedAt = now; return Decrease(step); }
                }
                return 0;
            }
            if (window == increaseButton)
            {
                if (ButtonState(window) == 2)
                {
                    const DWORD now = timeGetTime();
                    if (holdStartedAt + holdDelay < now && repeatedAt + repeatDelay < now)
                    { repeatedAt = now; return Increase(step); }
                }
                return 0;
            }
            if (window == track)
            {
                if (ButtonState(window) == 2)
                {
                    const DWORD now = timeGetTime();
                    if (holdStartedAt + holdDelay < now && repeatedAt + repeatDelay < now)
                    {
                        repeatedAt = now;
                        MouseState mouse;
                        g_Manager.GetMouseState(mouse);
                        const LONG direction = DirectionAt(mouse.x, mouse.y);
                        if (direction == pressedDirection)
                        {
                            switch (direction)
                            {
                            case -1: return Increase(pageStep);
                            case 1: return Decrease(pageStep);
                            }
                        }
                    }
                }
                return 0;
            }
            if (window == thumb && dragging)
            {
                MouseState mouse;
                g_Manager.GetMouseState(mouse);
                if (mouse.buttons & 1) return DragTo(mouse.x, mouse.y);
                const LONG current = value;
                dragging = 0;
                SetValue(current, 1);
            }
            return 0;
        }

        if (window->type != 1) return 0;
        if (window == decreaseButton)
        {
            const DWORD now = timeGetTime();
            holdStartedAt = now; repeatedAt = now;
            Decrease(step);
            return 1; //Consumed even when already at the lower limit.
        }
        if (window == increaseButton)
        {
            const DWORD now = timeGetTime();
            const LONG capturedStep = step;
            holdStartedAt = now; repeatedAt = now;
            Increase(capturedStep);
            return 1;
        }
        if (window == track)
        {
            const DWORD now = timeGetTime();
            holdStartedAt = now; repeatedAt = now;
            const LONG direction = DirectionAt(event->x, event->y);
            pressedDirection = direction;
            switch (direction)
            {
            case -1: Increase(pageStep); break;
            case 1: Decrease(pageStep); break;
            }
            return 1;
        }
        Window* capturedThumb = thumb;
        if (window == capturedThumb)
        {
            LONG x, y;
            capturedThumb->GetAbsolutePosition(x, y);
            dragOffsetX = LONG(DWORD(event->x) - DWORD(x));
            dragOffsetY = LONG(DWORD(event->y) - DWORD(y));
            dragging = 1;
        }
        return 0; //Starting a thumb drag does not report handled1.
    }
}
