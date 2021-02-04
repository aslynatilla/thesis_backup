#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine{

    class PointLight{
    public:
        explicit PointLight(glm::vec3 light_position) noexcept;
        glm::vec3 position;

        //  consider something that returns directions to bind
        //  to GL_CUBEMAP_POSITIVE_Z/GL_CUBEMAP_NEGATIVE_Z etc.
        //  std::array<glm::vec3, 6> cubemap_directios() const;
    };

}

#endif //POINT_LIGHT_H
