#ifndef OPENGL3_CUBEMAP_H
#define OPENGL3_CUBEMAP_H

#include "opengl3_texture.h"
#include <glm/glm.hpp>
#include <array>

namespace engine {
    class OpenGL3_Cubemap {
    public:
        OpenGL3_Cubemap(const GLenum image_format,
                        const OpenGL3_TextureParameters& parameters,
                        const unsigned int texture_width, const unsigned int texture_height,
                        const GLenum data_format, const GLenum data_type, const void* data);
        ~OpenGL3_Cubemap();
        OpenGL3_Cubemap(const OpenGL3_Cubemap& other) = delete;
        OpenGL3_Cubemap(OpenGL3_Cubemap&& other) = delete;
        OpenGL3_Cubemap& operator=(const OpenGL3_Cubemap& other) = delete;
        OpenGL3_Cubemap& operator=(OpenGL3_Cubemap&& other) = delete;

        void bind_to_slot(const unsigned int slot);

        unsigned int id;
        GLenum bound_type;
        unsigned int width;
        unsigned int height;

        static constexpr std::array<glm::vec3, 6> directions = std::array<glm::vec3, 6>{
            glm::vec3{1.0f, 0.0f, 0.0f},
            glm::vec3{-1.0f, 0.0f, 0.0f},
            glm::vec3{0.0f, 1.0f, 0.0f},
            glm::vec3{0.0f, -1.0f, 0.0f},
            glm::vec3{0.0f, 0.0f, 1.0f},
            glm::vec3{0.0f, 0.0f, -1.0f},
        };

        static constexpr std::array<glm::vec3, 6> ups = std::array<glm::vec3, 6>{
                glm::vec3{0.0f, -1.0f, 0.0f},
                glm::vec3{0.0f, -1.0f, 0.0f},
                glm::vec3{0.0f, 0.0f, 1.0f},
                glm::vec3{0.0f, 0.0f, -1.0f},
                glm::vec3{0.0f, -1.0f, 0.0f},
                glm::vec3{0.0f, -1.0f, 0.0f},
        };
    };
}

#endif //OPENGL3_CUBEMAP_H
