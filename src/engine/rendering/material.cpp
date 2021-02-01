#include "material.h"

namespace engine{
    std::string Material::default_name = "Unnamed Material";

    void Material::bind_uniforms_to(std::shared_ptr<Shader> shader) const {
        shader->use();
        shader->set_float("opacity", opacity);
        shader->set_float("shininess", shininess);
        shader->set_float("refract_i", refraction_index);
        shader->set_vec4("diffuse_color", diffuse_color);
        shader->set_vec4("ambient_color", ambient_color);
        shader->set_vec4("specular_color", specular_color);
        shader->set_vec4("emissive_color", emissive_color);
        shader->set_vec4("transparent_color", transparent_color);
    }
}