#include "glfw_window_impl.h"

namespace engine {

    static uint8_t app_window_count = 0;

    GLFW_Window_Impl::GLFW_Window_Impl(const WindowProperties& properties) {
        initialize(properties);
    }

    GLFW_Window_Impl::~GLFW_Window_Impl() {
        close();
    }

    void GLFW_Window_Impl::on_update() {
        glfwPollEvents();
        context->swap_buffers();
    }

    uint32_t GLFW_Window_Impl::get_width() const noexcept {
        return data.width;
    }

    uint32_t GLFW_Window_Impl::get_height() const noexcept {
        return data.height;
    }

    void *GLFW_Window_Impl::get_native_window() const noexcept {
        return window;
    }

    void GLFW_Window_Impl::set_event_callback(const Window::CallbackFunction& callback) noexcept {
        data.callback_func = callback;
    }

    void GLFW_Window_Impl::initialize(const WindowProperties& properties) {
        data.title = properties.title;
        data.width = properties.width;
        data.height = properties.height;

        if (app_window_count == 0) {
            int successful_initialization = glfwInit();
            if (!successful_initialization) {
                fmt::print(fg(fmt::color::red), "[GLFW WINDOW] Failed to create GLFW window.\n");
                glfwTerminate();
            }
            //  check how hints are set here, else enable this
            // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }

        window = glfwCreateWindow(static_cast<int>(properties.width),
                                  static_cast<int>(properties.height),
                                  properties.title.c_str(),
                                  nullptr, nullptr);
        app_window_count++;

        context = RenderingContext::create(window);
        context->initialize();
        glfwSetWindowUserPointer(window, &data);
        glfwSwapInterval(1);

        glfwSetWindowSizeCallback(window, window_resize_callback);
        glfwSetWindowCloseCallback(window, window_close_callback);
        glfwSetKeyCallback(window, key_callback);
        glfwSetCharCallback(window, key_typed_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetScrollCallback(window, mouse_scroll_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
    }

    void GLFW_Window_Impl::close() {
        glfwDestroyWindow(window);
        app_window_count--;

        if (app_window_count == 0) {
            glfwTerminate();
        }
    }

    void GLFW_Window_Impl::window_resize_callback(GLFWwindow *target_window,
                                                  int new_width, int new_height) {
        GLFW_Window_Data& target_window_data =
                *(static_cast<GLFW_Window_Data *>(glfwGetWindowUserPointer(target_window)));
        target_window_data.width = new_width;
        target_window_data.height = new_height;

        WindowResizedEvent resizing_event(static_cast<uint16_t>(new_width), static_cast<uint16_t>(new_height));
        target_window_data.callback_func(resizing_event);
    }

    void GLFW_Window_Impl::window_close_callback(GLFWwindow *target_window) {
        GLFW_Window_Data& target_window_data =
                *(static_cast<GLFW_Window_Data *>(glfwGetWindowUserPointer(target_window)));

        WindowClosedEvent closing_event;
        target_window_data.callback_func(closing_event);
    }

    void GLFW_Window_Impl::key_callback(GLFWwindow *target_window,
                                        int key,
                                        [[maybe_unused]] int scancode,
                                        int action,
                                        [[maybe_unused]] int modifiers) {
        GLFW_Window_Data& target_window_data =
                *(static_cast<GLFW_Window_Data *>(glfwGetWindowUserPointer(target_window)));

        switch (action) {
            case GLFW_PRESS: {
                KeyPressedEvent press(key, false);
                target_window_data.callback_func(press);

                if(key == GLFW_KEY_ESCAPE){
                    WindowClosedEvent close_window;
                    target_window_data.callback_func(close_window);
                }
                break;
            }
            case GLFW_RELEASE: {
                KeyReleasedEvent release(key);
                target_window_data.callback_func(release);
                break;
            }
            case GLFW_REPEAT: {
                KeyPressedEvent held_down(key, true);
                target_window_data.callback_func(held_down);
                break;
            }
        }
    }

    void GLFW_Window_Impl::key_typed_callback(GLFWwindow *target_window, [[maybe_unused]] unsigned int key) {
        GLFW_Window_Data& target_window_data =
                *(static_cast<GLFW_Window_Data *>(glfwGetWindowUserPointer(target_window)));

        KeyTypedEvent typed(key);
        target_window_data.callback_func(typed);
    }

    void GLFW_Window_Impl::mouse_button_callback(GLFWwindow *target_window,
                                                 int button, int action,
                                                 [[maybe_unused]] int modifiers) {
        GLFW_Window_Data& target_window_data =
                *(static_cast<GLFW_Window_Data *>(glfwGetWindowUserPointer(target_window)));

        switch (action) {
            case GLFW_PRESS: {
                MouseButtonPressedEvent button_press(button);
                target_window_data.callback_func(button_press);
                break;
            }
            case GLFW_RELEASE: {
                MouseButtonReleasedEvent button_release(button);
                target_window_data.callback_func(button_release);
                break;
            }
        }
    }

    void GLFW_Window_Impl::mouse_scroll_callback(GLFWwindow *target_window,
                                                 double x_scroll, double y_scroll) {
        GLFW_Window_Data& target_window_data =
                *(static_cast<GLFW_Window_Data *>(glfwGetWindowUserPointer(target_window)));

        MouseScrolledEvent scrolling(static_cast<float>(x_scroll), static_cast<float>(y_scroll));
        target_window_data.callback_func(scrolling);
    }

    void GLFW_Window_Impl::cursor_position_callback(GLFWwindow *target_window,
                                                    double x_coord, double y_coord) {
        GLFW_Window_Data& target_window_data =
                *(static_cast<GLFW_Window_Data *>(glfwGetWindowUserPointer(target_window)));

        MouseMovedEvent moving(static_cast<float>(x_coord), static_cast<float>(y_coord));
        target_window_data.callback_func(moving);
    }
}


