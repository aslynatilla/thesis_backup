#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace engine{
    struct MaterialData{
        alignas(16) glm::vec4 diffuse_color;
        alignas(16) glm::vec4 ambient_color;
        alignas(16) glm::vec4 specular_color;
        alignas(16) glm::vec4 emissive_color;
        alignas(16) glm::vec4 transparent_color;
        alignas(4) float opacity;
        alignas(4) float shininess;
        alignas(4) float refraction_index;

        [[nodiscard]] const void* raw() const;
    };

    struct Material{
    public:
        std::string name;
        MaterialData data;

        static std::string default_name;
    };
}

#endif //MATERIAL_H
