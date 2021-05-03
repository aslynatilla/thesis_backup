#include "point_light.h"

namespace engine {
    const void* PointLightData::raw() const {
        return static_cast<const void*>(this);
    }

    Point_Light::Point_Light(const glm::vec4 light_position,
                             const LightAttenuationParameters attenuation_descr) noexcept
            : Light(light_position, attenuation_descr) {}

    PointLightData Point_Light::get_representative_data() const {
        return PointLightData{
                .position = position,
                .direction = get_forward(),
                .constant_attenuation = attenuation.constant,
                .linear_attenuation = attenuation.linear,
                .quadratic_attenuation = attenuation.quadratic
        };
    }

}