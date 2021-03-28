#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine {
    struct SpotlightParameters {
        SpotlightParameters() = default;
        SpotlightParameters(const float cutoff_angle_degrees, const float outer_cutoff_angle_degrees);

        float cutoff_angle;
        float outer_cutoff_angle;

        float cosine_cutoff_angle;
        float cosine_outer_cutoff_angle;
    };

    struct LightAttenuationParameters {
        float constant;
        float linear;
        float quadratic;
    };

    class Spotlight {
    public:
        Spotlight() = default;
        explicit Spotlight(const glm::vec4 start_position,
                           const SpotlightParameters spot_descr,
                           const LightAttenuationParameters attenuation_descr) noexcept;

        glm::vec4 get_position() const;
        glm::vec3 get_rotation_in_degrees() const;
        glm::vec4 get_forward() const;
        glm::vec3 get_position_as_vec3() const;
        glm::vec3 get_looked_at_point() const;
        glm::fquat get_orientation() const;

        void translate_to(const glm::vec4 new_position);
        void set_rotation(const glm::vec3 new_rotation_in_degrees);

        SpotlightParameters spot_params;
        LightAttenuationParameters attenuation;
    private:
        glm::vec4 position;
        glm::fquat orientation;
        glm::vec3 rotation_in_degrees;
    };

}


#endif //SPOTLIGHT_H
