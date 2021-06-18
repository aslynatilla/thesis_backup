#include "window_events.h"

namespace engine {

    std::string WindowClosedEvent::to_string() const {
        return std::string("Window Closed Event");
    }

    WindowResizedEvent::WindowResizedEvent(unsigned width, unsigned height)
            : new_width{width}, new_height{height} {}

    unsigned WindowResizedEvent::get_target_width() const {
        return new_width;
    }

    unsigned WindowResizedEvent::get_target_height() const {
        return new_height;
    }

    std::string WindowResizedEvent::to_string() const {
        return fmt::format("Window Resized Event - target size ({}, {})", new_width, new_height);
    }
}