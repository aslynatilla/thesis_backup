#ifndef GLFW_WINDOW_IMPL_H
#define GLFW_WINDOW_IMPL_H

#include "window.h"
#include "rendering_context.h"
#include "../events/window_events.h"
#include "../events/keyboard_events.h"
#include "../events/mouse_events.h"

#include <fmt/core.h>
#include <fmt/color.h>
#include <GLFW/glfw3.h>

namespace engine {
    class GLFW_Window_Impl : public Window {
    public:
        static GLFW_Window_Impl* convert_from(Window* abstract_window);

        explicit GLFW_Window_Impl(const WindowProperties& properties);

        virtual ~GLFW_Window_Impl() override;

        void on_update() override;

        [[nodiscard]] unsigned get_width() const noexcept override;

        [[nodiscard]] unsigned get_height() const noexcept override;

        [[nodiscard]] void *get_native_window() const noexcept override;

        void set_event_callback(const CallbackFunction& callback) noexcept override;

    private:
        virtual void initialize(const WindowProperties& properties);

        virtual void close();

        static void window_resize_callback(GLFWwindow *target_window, int new_width, int new_height);

        static void window_close_callback(GLFWwindow *target_window);

        static void key_callback(GLFWwindow *target_window, int key, int scancode, int action, int modifiers);

        static void key_typed_callback(GLFWwindow *target_window, unsigned int key);

        static void mouse_button_callback(GLFWwindow *target_window, int button, int action, int modifiers);

        static void mouse_scroll_callback(GLFWwindow *target_window, double x_scroll, double y_scroll);

        static void cursor_position_callback(GLFWwindow *target_window, double x_coord, double y_coord);

        GLFWwindow *window;
        std::unique_ptr<RenderingContext> context;

        struct GLFW_Window_Data {
            std::string title;
            uint32_t width;
            uint32_t height;

            CallbackFunction callback_func;
        };

        GLFW_Window_Data data;
    };
}

#endif //GLFW_WINDOW_IMPL_H
