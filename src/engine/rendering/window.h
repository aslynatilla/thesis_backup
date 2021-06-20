#ifndef WINDOW_H
#define WINDOW_H

#include <functional>
#include <memory>
#include <string>

#include "../events/event.h"

namespace engine {

    struct WindowProperties {
        std::string title;
        unsigned width;
        unsigned height;

        explicit WindowProperties(std::string_view w_title = "MainWindow",
                                  unsigned w_width = 800u,
                                  unsigned w_height = 600u)
                : title{w_title}, width{w_width}, height{w_height} {}
    };

    class Window {
    public:
        using EventQueueAccess = std::function<void(std::unique_ptr<Event>)>;

        virtual ~Window() = default;

        virtual void on_update() = 0;

        [[nodiscard]] virtual unsigned get_width() const = 0;

        [[nodiscard]] virtual unsigned get_height() const = 0;

        [[nodiscard]] virtual void *get_native_window() const = 0;

        virtual void link_to_event_queue(const EventQueueAccess& push_function) = 0;

        static std::unique_ptr<Window> create(const WindowProperties& w);
    };
}

#endif //WINDOW_H
