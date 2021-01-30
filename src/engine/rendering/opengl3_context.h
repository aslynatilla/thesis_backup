#ifndef OPENGL3_CONTEXT_H
#define OPENGL3_CONTEXT_H

#include "rendering_context.h"

#include <fmt/core.h>
#include <fmt/color.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct GLFWwindow;

namespace engine {
    class OpenGL3_Context : public RenderingContext {
    public:
        OpenGL3_Context(GLFWwindow *window);

        virtual void initialize() override;

        virtual void swap_buffers() override;

    private:
        GLFWwindow *main_window;
    };
}

#endif //OPENGL3_CONTEXT_H
