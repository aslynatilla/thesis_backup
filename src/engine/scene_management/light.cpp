#include "light.h"

namespace engine {
    Light::Light(const glm::vec4 start_position, const LightAttenuationParameters attenuation_descr)
            : position(start_position),
              orientation(glm::identity<glm::fquat>()),
              rotation_in_degrees(0.0f),
              attenuation(attenuation_descr) {}

    glm::vec4 Light::get_position() const {
        return position;
    }

    glm::vec3 Light::get_rotation_in_degrees() const {
        return rotation_in_degrees;
    }

    glm::vec4 Light::get_forward() const {
        return glm::mat4_cast(orientation) * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    }

    glm::vec4 Light::get_up() const {
        return glm::mat4_cast(orientation) * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    }

    glm::vec3 Light::get_position_as_vec3() const {
        return glm::vec3(position);
    }

    glm::fquat Light::get_orientation() const {
        return orientation;
    }

    void Light::translate_to(const glm::vec4 new_position) {
        position = new_position;
    }

    void Light::set_rotation(const glm::vec3 new_rotation_in_degrees) {
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