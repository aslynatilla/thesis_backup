#ifndef OPENGL3_CUBEMAP_H
#define OPENGL3_CUBEMAP_H

#include "opengl3_texture.h"
#include <glm/glm.hpp>
#include <array>

namespace engine {
    class OpenGL3_Cubemap {
    public:
        friend class OpenGL3_Cubemap_Builder;
        [[nodiscard]] unsigned int id() const;
        void bind_to_slot(unsigned int slot) const;


        ~OpenGL3_Cubemap();
        OpenGL3_Cubemap(const OpenGL3_Cubemap& other) = delete;
        OpenGL3_Cubemap(OpenGL3_Cubemap&& other) = delete;
        OpenGL3_Cubemap& operator=(const OpenGL3_Cubemap& other) = delete;
        OpenGL3_Cubemap& operator=(OpenGL3_Cubemap&& other) = delete;


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
    private:
        unsigned int resource_id;
        GLenum bound_type;
        unsigned int width;
        unsigned int height;

        OpenGL3_Cubemap(unsigned int texture_id,
                          unsigned int texture_width, unsigned int texture_height);
    };

    class OpenGL3_Cubemap_Builder : public OpenGL3_Texture_Builder<OpenGL3_Cubemap_Builder> {
    public:
        OpenGL3_Cubemap_Builder() = default;
        [[nodiscard]] OpenGL3_Cubemap_Builder&& with_size(int tex_width, int tex_height) &&;
        [[nodiscard]] OpenGL3_Cubemap_Builder&& using_clamping_to_borders() &&;
        [[nodiscard]] OpenGL3_Cubemap_Builder&& using_clamping_to_edge() &&;

        [[nodiscard]] std::unique_ptr<OpenGL3_Cubemap> as_resource();
    private:
        int width = 0;
        int height = 0;
    };
}

#endif //OPENGL3_CUBEMAP_H
