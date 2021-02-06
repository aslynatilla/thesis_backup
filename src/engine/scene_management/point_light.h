#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine{

    class PointLight{
    public:
        PointLight() = default;
        explicit PointLight(glm::vec3 light_position) noexcept;
        glm::vec3 position;
    };

}

#endif //POINT_LIGHT_H
