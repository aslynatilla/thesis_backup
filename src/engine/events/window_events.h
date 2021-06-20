#ifndef WINDOW_EVENTS_H
#define WINDOW_EVENTS_H

#include "event.h"
#include "../rendering/window.h"

namespace engine {
    class WindowClosedEvent : public EventImpl<EventType::WindowClosed> {
    public:
        [[nodiscard]] std::string to_string() const override;
    };

    class WindowResizedEvent : public EventImpl<EventType::WindowResized> {
    public:
        WindowResizedEvent(unsigned width, unsigned height);

        [[nodiscard]] unsigned get_target_width() const;

        [[nodiscard]] unsigned get_target_height() const;

        [[nodiscard]] std::string to_string() const override;

    private:
        unsigned new_width;
        unsigned new_height;
    };
}

#endif //WINDOW_EVENTS_H
