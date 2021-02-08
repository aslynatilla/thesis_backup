#ifndef RENDERER_H
#define RENDERER_H

#include "vertex_array.h"
#include "shader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace engine{
    class OpenGL3_Renderer{
    public:
        static void initialize();

        static void clear(const int bitmask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        static void set_clear_color(const glm::vec4& clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
        static void set_clear_color(const float r, const float g, const float b, const float a);

        static void set_viewport(const unsigned int origin_x, const unsigned int origin_y,
                                 const unsigned int width, const unsigned int height);

        static void draw(const VertexArray& vao);

        static unsigned int register_shader(std::unique_ptr<Shader>&& shader_to_register);
        static Shader* get_registered_shader(unsigned int shader_index);

        static std::vector<std::unique_ptr<Shader>> loaded_shaders;

    };

}

#endif //RENDERER_H
