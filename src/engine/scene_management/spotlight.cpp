#include "spotlight.h"

namespace engine {
    SpotlightParameters::SpotlightParameters(const float cutoff_angle_degrees, const float outer_cutoff_angle_degrees)
            : cutoff_angle(cutoff_angle_degrees),
              outer_cutoff_angle(outer_cutoff_angle_degrees),
              cosine_cutoff_angle(glm::cos(glm::radians(cutoff_angle_degrees))),
              cosine_outer_cutoff_angle(glm::cos(glm::radians(outer_cutoff_angle_degrees))){}

    Spotlight::Spotlight(const glm::vec4 start_position,
                         const SpotlightParameters spot_descr,
                         const LightAttenuationParameters attenuation_descr) noexcept
            : position(start_position),
              spot_params(spot_descr),
              attenuation(attenuation_descr),
              rotation_in_degrees(0.0f),
              orientation(glm::identity<glm::mat4>()) {}

    glm::vec4 Spotlight::get_position() const {
        return position;
    }

    glm::vec3 Spotlight::get_rotation_in_degrees() const {
        return rotation_in_degrees;
    }

    glm::vec4 Spotlight::get_forward() const {
        return glm::mat4_cast(orientation) * glm::vec4(0.0f, 0.0f, 1.0f, 0.0);
    }

    glm::vec3 Spotlight::get_position_as_vec3() const {
        return glm::vec3(position);
    }

    glm::vec3 Spotlight::get_looked_at_point() const {
        return glm::vec3(position) + glm::vec3(get_forward());
    }

    glm::fquat Spotlight::get_orientation() const {
        return orientation;
    }

    void Spotlight::translate_to(const glm::vec4 new_position) {
        position = new_position;
    }

    void Spotlight::set_rotation(const glm::vec3 new_rotation_in_degrees) {
        const auto old_angles = glm::radians(rotation_in_degrees);
        const auto new_angles = glm::radians(new_rotation_in_degrees);
        rotation_in_degrees = new_rotation_in_degrees;

        const auto inverse_rotation_X = glm::angleAxis(-old_angles.x, glm::vec3(1.0f, 0.0f, 0.0f));
        const auto inverse_rotation_Y = glm::angleAxis(-old_angles.y, glm::vec3(0.0f, 1.0f, 0.0f));
        const auto inverse_rotation_Z = glm::angleAxis(-old_angles.z, glm::vec3(0.0f, 0.0f, 1.0f));
        const auto rotation_X = glm::angleAxis(new_angles.x, glm::vec3(1.0f, 0.0f, 0.0f));
        const auto rotation_Y = glm::angleAxis(new_angles.y, glm::vec3(0.0f, 1.0f, 0.0f));
        const auto rotation_Z = glm::angleAxis(new_angles.z, glm::vec3(0.0f, 0.0f, 1.0f));

        orientation = inverse_rotation_X * inverse_rotation_Y * inverse_rotation_Z * orientation;
        orientation = rotation_Z * rotation_Y * rotation_X * orientation;
        orientation = glm::normalize(orientation);
    }
}