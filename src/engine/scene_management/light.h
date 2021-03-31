#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine{
    struct LightAttenuationParameters {
        float constant;
        float linear;
        float quadratic;
    };

    class Light{
    public:
        virtual ~Light() = default;
        explicit Light(const glm::vec4 start_position,
                       const LightAttenuationParameters attenuation_descr);
        Light() = default;

        virtual glm::vec4 get_position() const;
        virtual glm::vec3 get_rotation_in_degrees() const;
        virtual glm::vec4 get_forward() const;
        virtual glm::vec4 get_up() const;
        virtual glm::vec3 get_position_as_vec3() const;
        virtual glm::fquat get_orientation() const;

        virtual void translate_to(const glm::vec4 new_position);
        virtual void set_rotation(const glm::vec3 new_rotation_in_degrees);

        LightAttenuationParameters attenuation;
    protected:
        glm::vec4 position;
        glm::fquat orientation;
        glm::vec3 rotation_in_degrees;
    };
}

#endif //LIGHT_H
