#include "window.h"

#include "glfw_window_impl.h"

namespace engine {
    std::unique_ptr<Window> Window::create(const WindowProperties& w) {
        return std::make_unique<GLFW_Window_Impl>(w);
    }
}