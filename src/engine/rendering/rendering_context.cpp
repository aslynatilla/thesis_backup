#include "rendering_context.h"

#include "opengl3_context.h"

namespace engine {
    RenderingContext::~RenderingContext() {
    }

    std::unique_ptr<RenderingContext> RenderingContext::create(void *window) {
        return std::make_unique<OpenGL3_Context>(static_cast<GLFWwindow *>(window));
    }
}