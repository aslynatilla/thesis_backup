#ifndef MESH_INTERPOLATION_H
#define MESH_INTERPOLATION_H

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include <vector>

namespace ies::adapter{
    using vec2_grid = std::vector<std::vector<glm::vec2>>;
    using vec3_grid = std::vector<std::vector<glm::vec3>>;

    vec3_grid interpolate_grid(const vec2_grid& angles, vec3_grid&& points, uint16_t new_points_per_edge);

    namespace impl_details{
        struct GridDimension{
            unsigned width;
            unsigned height;

            static GridDimension from(const vec3_grid& target_grid);
        };
    }
}

#endif //MESH_INTERPOLATION_H
