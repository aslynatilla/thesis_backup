#include "mesh_interpolation.h"

namespace ies::adapter{
    vec3_grid interpolate_grid(const vec2_grid& angles, vec3_grid&& points, [[maybe_unused]] uint16_t new_points_per_edge) {
        using namespace impl_details;
        [[maybe_unused]] const auto dimensions = GridDimension::from(points);
        return ies::adapter::vec3_grid();
    }
};

namespace ies::adapter::impl_details{
    GridDimension GridDimension::from(const vec3_grid& target_grid) {
        return GridDimension{.width = target_grid[0].size(),
                             .height = target_grid.size()};
    }
}