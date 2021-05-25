#ifndef MESH_INTERPOLATION_H
#define MESH_INTERPOLATION_H

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include <algorithm>
#include <iterator>
#include <vector>

namespace ies::adapter {
    using vec2_grid = std::vector<std::vector<glm::vec2>>;
    using vec3_grid = std::vector<std::vector<glm::vec3>>;

    vec3_grid interpolate_grid(const vec2_grid& angles, vec3_grid&& points, uint16_t new_points_per_edge);


    namespace impl_details {
        struct GridDimension {
            unsigned width;
            unsigned height;

            static GridDimension from(const vec3_grid& target_grid);
        };

        template<typename T>
        struct Rect {
            T p00;
            T p01;
            T p10;
            T p11;
        };

        struct BlockDescriptor {
            Rect<glm::vec2> block_corners;
            Rect<glm::vec3> f_in_corners;

            [[nodiscard]] glm::vec2 compute_interpolation_step(unsigned short points_per_x_edge,
                                                 unsigned short points_per_y_edge) const;
        };

        std::vector<glm::vec3>
        reserve_block_space(bool is_last_row_block, bool is_last_column_block, uint16_t new_points_per_edge);

        void last_interpolated_block(BlockDescriptor block_info, unsigned short new_points,
                                     std::vector<glm::vec3>& block);

        void interpolated_block_on_last_col(BlockDescriptor block_info, unsigned short new_points,
                                            std::vector<glm::vec3>& block);

        void interpolated_block_on_last_row(BlockDescriptor block_info, unsigned short new_points,
                                            std::vector<glm::vec3>& block);

        void interpolated_block(BlockDescriptor block_info, unsigned short new_points,
                                std::vector<glm::vec3>& block);

        vec3_grid compose_interpolated_grid(GridDimension dimension, uint16_t new_points_per_edge, vec3_grid&& blocks);
    }
}

#endif //MESH_INTERPOLATION_H
