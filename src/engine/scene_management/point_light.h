#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "light.h"

namespace engine {

    class Point_Light : public Light{
    public:
        Point_Light() = default;

        explicit Point_Light(const glm::vec4 light_position,
                             const LightAttenuationParameters attenuation_descr) noexcept;
    };

}

#endif //POINT_LIGHT_H
