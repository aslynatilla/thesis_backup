#include "opengl3_context.h"

namespace engine {

    OpenGL3_Context::OpenGL3_Context(GLFWwindow *window) : main_window{window} {}

    void OpenGL3_Context::initialize() {
        glfwMakeContextCurrent(main_window);

        int initialization_status = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
        if (initialization_status == 0) {
            fmt::print(fg(fmt::color::red), "[OPENGL3 CONTEXT] Failed to initialize GLAD\n");
        }
    }

    void OpenGL3_Context::swap_buffers() {
        glfwSwapBuffers(main_window);
    }
}