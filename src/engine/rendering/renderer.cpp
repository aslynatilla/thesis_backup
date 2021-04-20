#include "renderer.h"

namespace engine {
    std::vector<std::unique_ptr<Shader>> OpenGL3_Renderer::loaded_shaders;

    void OpenGL3_Renderer::initialize() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
    }

    void OpenGL3_Renderer::clear(const int bitmask) {
        glClear(bitmask);
    }

    void OpenGL3_Renderer::set_clear_color(const glm::vec4& clear_color) {
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    }

    void OpenGL3_Renderer::set_clear_color(const float r, const float g, const float b, const float a) {
        glClearColor(r, g, b, a);
    }

    void OpenGL3_Renderer::set_viewport(const unsigned int origin_x, const unsigned int origin_y,
                                        const unsigned int width, const unsigned int height) {
        glViewport(origin_x, origin_y, width, height);
    }

    void OpenGL3_Renderer::draw(const VertexArray& vao) {
        vao.bind();
        glDrawElements(GL_TRIANGLES, vao.get_ebo()->get_count(), GL_UNSIGNED_INT, nullptr);
        vao.unbind();
    }

    unsigned int OpenGL3_Renderer::register_shader(std::unique_ptr<Shader>&& shader_to_register) {
        loaded_shaders.push_back(std::move(shader_to_register));
        return loaded_shaders.size() - 1;
    }

    Shader* OpenGL3_Renderer::get_registered_shader(unsigned int shader_index) {
        return loaded_shaders[shader_index].get();
    }

}