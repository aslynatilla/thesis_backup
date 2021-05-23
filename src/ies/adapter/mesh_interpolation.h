#ifndef MESH_INTERPOLATION_H
#define MESH_INTERPOLATION_H

#include "glm/vec3.hpp"
#include <vector>

namespace ies::adapter{
    using vec_grid = std::vector<std::vector<glm::vec3>>;
    vec_grid interpolate_grid(vec_grid&& starting_grid, uint16_t new_points_per_edge);
}

#endif //MESH_INTERPOLATION_H
