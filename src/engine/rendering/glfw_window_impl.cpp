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

    void GLFW_Window_Impl::link_to_event_queue(const Window::EventQueueAccess& push_function) noexcept {
        data.event_pump_function = push_function;
    }

    void GLFW_Window_Impl::initialize(const WindowProperties& properties) {
        data.title = properties.title;
        data.width = properties.width;
        data.height = properties.height;

        if (app_window_count == 0) {
            int successful_initialization = glfwInit();
            if (!successful_initialization) {
                fmt::print("[GLFW WINDOW] Failed to create GLFW window.\n");
                glfwTerminate();
            }

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }

        window = glfwCreateWindow(static_cast<int>(properties.width),
                                  static_cast<int>(properties.height),
                                  properties.title.c_str(),
                                  nullptr, nullptr);
        app_window_count++;

        context = RenderingContext::create(window);
        context->initialize();
        glfwSetWindowUserPointer(window, &data);
        glfwSwapInterval(0);

        glfwSetWindowSizeCallback(window, window_resize_callback);
        glfwSetWindowCloseCallback(window, window_close_callback);
        glfwSetKeyCallback(window, key_callback);
        glfwSetCharCallback(window, key_typed_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetScrollCallback(window, mouse_scroll_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);

        fmt::print("[GLFW WINDOW IMPLEMENTATION] Creation of OpenGL context, version {}, was successful.\n",
                   glGetString(GL_VERSION));
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

        target_window_data.event_pump_function(std::make_unique<WindowResizedEvent>(new_width, new_height));
    }

    void GLFW_Window_Impl::window_close_callback(GLFWwindow *target_window) {
        GLFW_Window_Data& target_window_data =
                *(static_cast<GLFW_Window_Data *>(glfwGetWindowUserPointer(target_window)));

        target_window_data.event_pump_function(std::make_unique<WindowClosedEvent>());
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
                target_window_data.event_pump_function(std::make_unique<KeyPressedEvent>(key, false));

                if(key == GLFW_KEY_ESCAPE){
                    target_window_data.event_pump_function(std::make_unique<WindowClosedEvent>());
                }
                break;
            }
            case GLFW_RELEASE: {
                target_window_data.event_pump_function(std::make_unique<KeyReleasedEvent>(key));
                break;
            }
            case GLFW_REPEAT: {
                target_window_data.event_pump_function(std::make_unique<KeyPressedEvent>(key, true));
                break;
            }
        }
    }

    void GLFW_Window_Impl::key_typed_callback(GLFWwindow *target_window, [[maybe_unused]] unsigned int key) {
        GLFW_Window_Data& target_window_data =
                *(static_cast<GLFW_Window_Data *>(glfwGetWindowUserPointer(target_window)));

        target_window_data.event_pump_function(std::make_unique<KeyTypedEvent>(key));
    }

    void GLFW_Window_Impl::mouse_button_callback(GLFWwindow *target_window,
                                                 int button, int action,
                                                 [[maybe_unused]] int modifiers) {
        GLFW_Window_Data& target_window_data =
                *(static_cast<GLFW_Window_Data *>(glfwGetWindowUserPointer(target_window)));

        switch (action) {
            case GLFW_PRESS: {
                target_window_data.event_pump_function(std::make_unique<MouseButtonPressedEvent>(button));
                break;
            }
            case GLFW_RELEASE: {
                target_window_data.event_pump_function(std::make_unique<MouseButtonReleasedEvent>(button));
                break;
            }
        }
    }

    void GLFW_Window_Impl::mouse_scroll_callback(GLFWwindow *target_window,
                                                 double x_scroll, double y_scroll) {
        GLFW_Window_Data& target_window_data =
                *(static_cast<GLFW_Window_Data *>(glfwGetWindowUserPointer(target_window)));

        target_window_data.event_pump_function(std::make_unique<MouseScrolledEvent>(
                static_cast<float>(x_scroll), static_cast<float>(y_scroll))
                );
    }

    void GLFW_Window_Impl::cursor_position_callback(GLFWwindow *target_window,
                                                    double x_coord, double y_coord) {
        GLFW_Window_Data& target_window_data =
                *(static_cast<GLFW_Window_Data *>(glfwGetWindowUserPointer(target_window)));

        target_window_data.event_pump_function(std::make_unique<MouseMovedEvent>(
                static_cast<float>(x_coord), static_cast<float>(y_coord)
                ));
    }

    GLFW_Window_Impl* GLFW_Window_Impl::convert_from(Window* abstract_window) {
        return dynamic_cast<GLFW_Window_Impl*>(abstract_window);
    }
}


