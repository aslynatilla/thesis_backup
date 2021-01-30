#include "window_events.h"

namespace engine {

    std::string WindowClosedEvent::to_string() const {
        return std::string("Window Closed Event");
    }

    WindowResizedEvent::WindowResizedEvent(uint16_t width, uint16_t height)
            : new_width{width}, new_height{height} {}

    uint16_t WindowResizedEvent::get_target_width() const {
        return new_width;
    }

    uint16_t WindowResizedEvent::get_target_height() const {
        return new_height;
    }

    std::string WindowResizedEvent::to_string() const {
        return fmt::format("Window Resized Event - target size ({}, {})", new_width, new_height);
    }
}