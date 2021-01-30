#ifndef WINDOW_EVENTS_H
#define WINDOW_EVENTS_H

#include "event.h"
#include "../rendering/window.h"

namespace engine {
    class WindowClosedEvent : public EventImpl<EventType::WindowClosed> {
    public:
        std::string to_string() const override;
    };

    class WindowResizedEvent : public EventImpl<EventType::WindowResized> {
    public:
        WindowResizedEvent(uint16_t width, uint16_t height);

        uint16_t get_target_width() const;

        uint16_t get_target_height() const;

        std::string to_string() const override;

    private:
        uint16_t new_width;
        uint16_t new_height;
    };
}

#endif //WINDOW_EVENTS_H
