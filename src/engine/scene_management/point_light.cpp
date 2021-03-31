#include "point_light.h"

namespace engine {
    Point_Light::Point_Light(const glm::vec4 light_position, const LightAttenuationParameters attenuation_descr) noexcept
            : Light(light_position, attenuation_descr)
            {}

}