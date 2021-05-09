#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "light.h"

namespace engine {

    struct PointLightData{
        alignas(16) glm::vec4 position;
        alignas(16) glm::vec4 direction;
        alignas(4)  float constant_attenuation;
        alignas(4)  float linear_attenuation;
        alignas(4)  float quadratic_attenuation;

        const void* raw() const;
    };

    class Point_Light : public Light{
    public:
        Point_Light() = default;

        explicit Point_Light(const glm::vec4 light_position,
                             const LightAttenuationParameters attenuation_descr) noexcept;

        PointLightData get_representative_data() const;
    };

}

#endif //POINT_LIGHT_H
