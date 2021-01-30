#ifndef IES_PARSER_WINDOW_H
#define IES_PARSER_WINDOW_H

#include <functional>
#include <memory>
#include <string>

#include "../events/event.h"

namespace engine {

    struct WindowProperties {
        std::string title;
        uint32_t width;
        uint32_t height;

        WindowProperties(const std::string& w_title = "MainWindow", uint32_t w_width = 800, uint32_t w_height = 600)
                : title{w_title}, width{w_width}, height{w_height} {}
    };

    class Window {
    public:
        using CallbackFunction = std::function<void(Event&)>;

        virtual ~Window() = default;

        virtual void on_update() = 0;

        virtual uint32_t get_width() const = 0;

        virtual uint32_t get_height() const = 0;

        virtual void *get_native_window() const = 0;

        virtual void set_event_callback(const CallbackFunction& callback) = 0;

        static std::unique_ptr<Window> create(const WindowProperties& w);
    };
}

#endif //IES_PARSER_WINDOW_H
