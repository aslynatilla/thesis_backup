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

    void Material::bind_uniforms_in_order(const int first_uniform_location, std::shared_ptr<Shader> shader_to_bind) const {
        shader_to_bind->use();
        shader_to_bind->set_float(first_uniform_location + 0, opacity);
        shader_to_bind->set_float(first_uniform_location + 1, shininess);
        shader_to_bind->set_float(first_uniform_location + 2, refraction_index);
        shader_to_bind->set_vec4(first_uniform_location + 3, diffuse_color);
        shader_to_bind->set_vec4(first_uniform_location + 4, ambient_color);
        shader_to_bind->set_vec4(first_uniform_location + 5, specular_color);
        shader_to_bind->set_vec4(first_uniform_location + 6, emissive_color);
        shader_to_bind->set_vec4(first_uniform_location + 7, transparent_color);
    }


}