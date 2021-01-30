#ifndef IES_PARSER_MOUSE_EVENTS_H
#define IES_PARSER_MOUSE_EVENTS_H

#include "event.h"

namespace engine {
    class MouseMovedEvent : public EventImpl<EventType::MouseMoved> {
    public:
        explicit MouseMovedEvent(const float x, const float y);
        [[nodiscard]] float x() const;
        [[nodiscard]] float y() const;
    private:
        float x_movement;
        float y_movement;
    };

    class MouseScrolledEvent : public EventImpl<EventType::MouseScrolled> {
    public:
        explicit MouseScrolledEvent(const float x_scroll, const float y_scroll);
        [[nodiscard]] float horizontal_scroll() const;
        [[nodiscard]] float vertical_scroll() const;
    private:
        float x_scrolling;
        float y_scrolling;
    };

    class MouseButtonPressedEvent : public EventImpl<EventType::MouseButtonPressed> {
    public:
        explicit MouseButtonPressedEvent(const int button_id);
        [[nodiscard]] int get_button() const;
    private:
        int button;
    };

    class MouseButtonReleasedEvent : public EventImpl<EventType::MouseButtonReleased> {
    public:
        explicit MouseButtonReleasedEvent(const int button_id);
        [[nodiscard]] int get_button() const;
    private:
        int button;
    };
}
#endif //IES_PARSER_MOUSE_EVENTS_H
