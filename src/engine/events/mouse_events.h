#ifndef MOUSE_EVENTS_H
#define MOUSE_EVENTS_H

#include "event.h"

namespace engine {
    class MouseMovedEvent : public EventImpl<EventType::MouseMoved> {
    public:
        explicit MouseMovedEvent(float x, float y);
        [[nodiscard]] float x() const;
        [[nodiscard]] float y() const;
    private:
        float x_movement;
        float y_movement;
    };

    class MouseScrolledEvent : public EventImpl<EventType::MouseScrolled> {
    public:
        explicit MouseScrolledEvent(float x_scroll, float y_scroll);
        [[nodiscard]] float horizontal_scroll() const;
        [[nodiscard]] float vertical_scroll() const;
    private:
        float x_scrolling;
        float y_scrolling;
    };

    class MouseButtonPressedEvent : public EventImpl<EventType::MouseButtonPressed> {
    public:
        explicit MouseButtonPressedEvent(int button_id);
        [[nodiscard]] int get_button() const;
    private:
        int button;
    };

    class MouseButtonReleasedEvent : public EventImpl<EventType::MouseButtonReleased> {
    public:
        explicit MouseButtonReleasedEvent(int button_id);
        [[nodiscard]] int get_button() const;
    private:
        int button;
    };
}
#endif //MOUSE_EVENTS_H
