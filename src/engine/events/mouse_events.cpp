#include "mouse_events.h"

namespace engine{

    MouseMovedEvent::MouseMovedEvent(float x, float y) : x_movement{x}, y_movement{y} {}

    float MouseMovedEvent::x() const {
        return this->x_movement;
    }

    float MouseMovedEvent::y() const {
        return this->y_movement;
    }

    MouseScrolledEvent::MouseScrolledEvent(float x_scroll, float y_scroll) : x_scrolling{x_scroll}, y_scrolling{y_scroll}{}

    float MouseScrolledEvent::horizontal_scroll() const {
        return this->x_scrolling;
    }

    float MouseScrolledEvent::vertical_scroll() const {
        return this->y_scrolling;
    }

    MouseButtonPressedEvent::MouseButtonPressedEvent(int button_id) : button{button_id}{}

    int MouseButtonPressedEvent::get_button() const {
        return this->button;
    }

    MouseButtonReleasedEvent::MouseButtonReleasedEvent(int button_id) : button{button_id} {}

    int MouseButtonReleasedEvent::get_button() const {
        return this->button;
    }
}