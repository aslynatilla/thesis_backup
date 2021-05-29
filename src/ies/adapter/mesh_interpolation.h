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

    vec3_grid
    interpolate_grid(const vec2_grid& domain_points, vec3_grid&& codomain_points, unsigned int new_points_per_edge);


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

            [[nodiscard]] glm::vec3 bilinear_interpolation_in(glm::vec2 point) const;
        };

        struct FloatRange {
            float start;
            float end;
        };

        [[nodiscard]] std::vector<float> compute_range(unsigned int samples_in_range,
                                                       FloatRange range_endpoints,
                                                       bool includes_range_end);

        [[nodiscard]] float compute_sampling_step(unsigned int number_of_steps,
                                                  FloatRange range_to_sample);

        [[nodiscard]] std::vector<glm::vec2> flat_cartesian_product(const std::vector<float>& first_values,
                                                                    const std::vector<float>& second_values);

        std::vector<glm::vec3>
        create_block_and_reserve(const unsigned int new_points_per_edge, const bool is_last_row_block,
                                 const bool is_last_column_block);

        void interpolated_block_last(GridQuad block_info, unsigned int new_points,
                                     std::vector<glm::vec3>& block);

        void interpolated_block_on_last_col(GridQuad block_info, unsigned int new_points,
                                            std::vector<glm::vec3>& block);

        void interpolated_block_on_last_row(GridQuad block_info, unsigned int new_points,
                                            std::vector<glm::vec3>& block);

        void interpolated_block(GridQuad block_info, unsigned int new_points,
                                std::vector<glm::vec3>& block);

        vec3_grid compose_interpolated_blocks_as_grid(GridDimension original_dimensions, unsigned new_points_per_edge,
                                                      vec3_grid&& blocks);
    }
}

#endif //MESH_INTERPOLATION_H
