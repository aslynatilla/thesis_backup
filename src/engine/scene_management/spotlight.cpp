#include "spotlight.h"

namespace engine {
    SpotlightParameters::SpotlightParameters(const float cutoff_angle_degrees, const float outer_cutoff_angle_degrees)
            : cutoff_angle(cutoff_angle_degrees),
              outer_cutoff_angle(outer_cutoff_angle_degrees),
              cosine_cutoff_angle(glm::cos(glm::radians(cutoff_angle_degrees))),
              cosine_outer_cutoff_angle(glm::cos(glm::radians(outer_cutoff_angle_degrees))) {}

    Spotlight::Spotlight(const glm::vec4 start_position,
                         const LightAttenuationParameters attenuation_descr,
                         const SpotlightParameters spot_descr) noexcept
            : Light(start_position, attenuation_descr),
              spot_params(spot_descr) {}

    glm::vec3 Spotlight::get_looked_at_point() const {
        return glm::vec3(position) + glm::vec3(get_forward());
    }
}