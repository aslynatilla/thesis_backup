#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "light.h"

namespace engine {
    struct SpotlightParameters {
        SpotlightParameters() = default;

        SpotlightParameters(const float cutoff_angle_degrees, const float outer_cutoff_angle_degrees);

        float cutoff_angle;
        float outer_cutoff_angle;

        float cosine_cutoff_angle;
        float cosine_outer_cutoff_angle;
    };

    class Spotlight : public Light {
    public:
        using Light::Light;
        Spotlight() = default;

        explicit Spotlight(const glm::vec4 start_position,
                           const LightAttenuationParameters attenuation_descr,
                           const SpotlightParameters spot_descr) noexcept;

        glm::vec3 get_looked_at_point() const;

        SpotlightParameters spot_params;
    };

}


#endif //SPOTLIGHT_H
