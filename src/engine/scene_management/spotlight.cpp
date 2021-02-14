#include "spotlight.h"

namespace engine{

    SpotLight::SpotLight(const glm::vec3 light_position, const glm::vec3 light_direction,
                         const float cutoff_angle_in_degrees, const float outer_cutoff_angle_in_degrees,
                         const float constant_att, const float linear_att, const float quadratic_att) noexcept :
                         position(light_position),
                         direction(light_direction),
                         cosine_cutoff_angle(glm::cos(glm::radians(cutoff_angle_in_degrees))),
                         cosine_outer_cutoff_angle(glm::cos(glm::radians(outer_cutoff_angle_in_degrees))),
                         constant_attenuation_factor(constant_att),
                         linear_attenuation_factor(linear_att),
                         quadratic_attenuation_factor(quadratic_att)
                         {}
}