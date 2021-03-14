#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine{

    class SpotLight{
    public:
        SpotLight() = default;
        explicit SpotLight(const glm::vec3 light_position, const glm::vec3 light_direction,
                           const float cutoff_angle_in_degrees, const float outer_cutoff_angle_in_degrees,
                           const float constant_att, const float linear_att, const float quadratic_att) noexcept;
        glm::vec3 position;
        glm::vec3 direction;

        float raw_cutoff_angle;
        float raw_outer_cutoff_angle;
        float cosine_cutoff_angle;
        float cosine_outer_cutoff_angle;
        float constant_attenuation_factor;
        float linear_attenuation_factor;
        float quadratic_attenuation_factor;
    };

}


#endif //SPOTLIGHT_H
