#ifndef MATERIAL_H
#define MATERIAL_H

#include "shader.h"

#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace engine{
    struct Material{
    public:
        std::string name;
        float opacity;
        float shininess;
        float refraction_index;
        glm::vec4 diffuse_color;
        glm::vec4 ambient_color;
        glm::vec4 specular_color;
        glm::vec4 emissive_color;
        glm::vec4 transparent_color;

        void bind_uniforms_to(std::shared_ptr<Shader> shader) const;
        void bind_uniforms_in_order(const int first_uniform_location, std::shared_ptr<Shader> shader_to_bind) const;

        static std::string default_name;
    };
}

#endif //MATERIAL_H
