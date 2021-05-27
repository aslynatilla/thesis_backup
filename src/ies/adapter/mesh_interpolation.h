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

    vec3_grid interpolate_grid(const vec2_grid& domain_points, vec3_grid&& codomain_points, unsigned int new_points_per_edge);


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

        struct GridQuad {
            Rect<glm::vec2> vertices;
            Rect<glm::vec3> f_in_vertex;

            [[nodiscard]] glm::vec2 compute_sampling_step(unsigned int steps_per_x_edge,
                                                          unsigned int steps_per_y_edge) const;

            [[nodiscard]] glm::vec3 bilinear_interpolation_in(const glm::vec2 point) const;
        };

        std::vector<glm::vec3>
        create_block_and_reserve(bool is_last_row_block, bool is_last_column_block, unsigned int new_points_per_edge);

        void interpolated_block_last(GridQuad block_info, unsigned int new_points,
                                     std::vector<glm::vec3>& block);

        void interpolated_block_on_last_col(GridQuad block_info, unsigned int new_points,
                                            std::vector<glm::vec3>& block);

        void interpolated_block_on_last_row(GridQuad block_info, unsigned int new_points,
                                            std::vector<glm::vec3>& block);

        void interpolated_block(GridQuad block_info, unsigned int new_points,
                                std::vector<glm::vec3>& block);

        vec3_grid compose_interpolated_blocks_as_grid(GridDimension original_dimensions, unsigned new_points_per_edge, vec3_grid&& blocks);
    }
}

#endif //MESH_INTERPOLATION_H
