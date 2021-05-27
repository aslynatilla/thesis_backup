#include "mesh_interpolation.h"

namespace ies::adapter {
    vec3_grid interpolate_grid(const vec2_grid& domain_points, vec3_grid&& codomain_points, unsigned int new_points_per_edge) {
        using namespace impl_details;
        const auto dimensions = GridDimension::from(codomain_points);
        const auto blocks_rows = dimensions.height - 1;
        const auto blocks_columns = dimensions.width - 1;

        //  Blocks are the sub-grids, the areas we build an interpolated function over
        //  Notice that blocks are flattened; a 5-by-5 block is a 25-long vector
        vec3_grid grid_of_blocks;
        grid_of_blocks.reserve(blocks_rows * blocks_columns);
        for (auto row = 0u; row < blocks_rows; ++row) {
            for (auto col = 0u; col < blocks_columns; ++col) {
                const bool is_last_row = (row == blocks_rows - 1);
                const bool is_last_column = (col == blocks_columns - 1);
                auto block = create_block_and_reserve(is_last_row, is_last_column, new_points_per_edge);

                GridQuad quad{
                        .vertices = {.p00 = domain_points[row][col],
                                .p01 = domain_points[row + 1][col],
                                .p10 = domain_points[row][col + 1],
                                .p11 = domain_points[row + 1][col + 1]},
                        .f_in_vertex = {.p00 = codomain_points[row][col],
                                .p01 = codomain_points[row + 1][col],
                                .p10 = codomain_points[row][col + 1],
                                .p11 = codomain_points[row + 1][col + 1]},

                };

                if (is_last_column && is_last_row) {
                    interpolated_block_last(quad, new_points_per_edge, block);
                } else if (is_last_column) {
                    interpolated_block_on_last_col(quad, new_points_per_edge, block);
                } else if (is_last_row) {
                    interpolated_block_on_last_row(quad, new_points_per_edge, block);
                } else {
                    interpolated_block(quad, new_points_per_edge, block);
                }
                grid_of_blocks.push_back(block);
            }
        }

        return compose_interpolated_blocks_as_grid(dimensions, new_points_per_edge, std::move(grid_of_blocks));
    }
};

namespace ies::adapter::impl_details {
    GridDimension GridDimension::from(const vec3_grid& target_grid) {
        return GridDimension{.width = target_grid[0].size(),
                .height = target_grid.size()};
    }

    //step
    glm::vec2
    GridQuad::compute_sampling_step(const unsigned int steps_per_x_edge,
                                    const unsigned int steps_per_y_edge) const {
        const auto horizontal_steps_between_vertices = static_cast<float>(steps_per_x_edge);
        const auto vertical_steps_between_vertices = static_cast<float>(steps_per_y_edge);
        return glm::vec2((vertices.p10.x - vertices.p00.x) / horizontal_steps_between_vertices,
                         (vertices.p01.y - vertices.p00.y) / vertical_steps_between_vertices);
    }

    //   TODO: this might be handled differently if using a struct to represent the block better;
    //  semantically, these should be two different methods, maybe chained...
    //  consider composition over inheritance and a struct containing these vectors
    std::vector<glm::vec3>
    create_block_and_reserve(const bool is_last_row_block, const bool is_last_column_block, const unsigned int new_points_per_edge) {
        const unsigned int points_per_generic_edge = new_points_per_edge + 1;
        const unsigned int points_per_last_edge = new_points_per_edge + 2;

        std::vector<glm::vec3> block;
        if (is_last_column_block && is_last_row_block) {
            block.reserve(points_per_last_edge * points_per_last_edge);
        } else if (is_last_column_block || is_last_row_block) {
            block.reserve(points_per_last_edge * points_per_generic_edge);
        } else {
            block.reserve(points_per_generic_edge * points_per_generic_edge);
        }
        return block;
    }

    void interpolated_block_last(GridQuad block_info, const unsigned int new_points,
                                 std::vector<glm::vec3>& block) {
        const unsigned int x_edge_points = new_points + 2;
        const unsigned int y_edge_points = new_points + 2;
        const unsigned int x_steps = new_points + 1;
        const unsigned int y_steps = new_points + 1;

        auto interpolator = [&block_info](const glm::vec2 interpolation_point) -> glm::vec3 {
            const auto& angles = block_info.vertices;
            const auto& f = block_info.f_in_vertex;
            const float denominator = (angles.p11.x - angles.p00.x) * (angles.p11.y - angles.p00.y);
            const auto term00 = f.p00 * (angles.p11.x - interpolation_point.x) * (angles.p11.y - interpolation_point.y);
            const auto term10 = f.p10 * (interpolation_point.x - angles.p00.x) * (angles.p11.y - interpolation_point.y);
            const auto term01 = f.p01 * (angles.p11.x - interpolation_point.x) * (interpolation_point.y - angles.p00.y);
            const auto term11 = f.p11 * (interpolation_point.x - angles.p00.x) * (interpolation_point.y - angles.p00.y);
            return (term00 + term01 + term10 + term11) / denominator;
        };

        const auto step = block_info.compute_sampling_step(x_steps, y_steps);
        for (auto i = 0u; i < y_edge_points; ++i) {
            auto y = (i == y_steps) ? block_info.vertices.p01.y
                                    : block_info.vertices.p00.y + step.y * static_cast<float>(i);
            for (auto j = 0u; j < x_edge_points; ++j) {
                auto x = (j == x_steps) ? block_info.vertices.p10.x
                                        : block_info.vertices.p00.x + step.x * static_cast<float>(j);
                block.push_back(interpolator(glm::vec2(x, y)));
            }
        }
    }

    void interpolated_block_on_last_col(GridQuad block_info, const unsigned int new_points,
                                        std::vector<glm::vec3>& block) {
        const unsigned int x_edge_points = new_points + 2;
        const unsigned int y_edge_points = new_points + 1;
        const unsigned int x_steps = new_points + 1;
        const unsigned int y_steps = new_points + 1;

        auto interpolator = [&block_info](const glm::vec2 interpolation_point) -> glm::vec3 {
            const auto& angles = block_info.vertices;
            const auto& f = block_info.f_in_vertex;
            const float denominator = (angles.p11.x - angles.p00.x) * (angles.p11.y - angles.p00.y);
            const auto term00 = f.p00 * (angles.p11.x - interpolation_point.x) * (angles.p11.y - interpolation_point.y);
            const auto term10 = f.p10 * (interpolation_point.x - angles.p00.x) * (angles.p11.y - interpolation_point.y);
            const auto term01 = f.p01 * (angles.p11.x - interpolation_point.x) * (interpolation_point.y - angles.p00.y);
            const auto term11 = f.p11 * (interpolation_point.x - angles.p00.x) * (interpolation_point.y - angles.p00.y);
            return (term00 + term01 + term10 + term11) / denominator;
        };

        const auto step = block_info.compute_sampling_step(x_steps, y_steps);
        for (auto i = 0u; i < y_edge_points; ++i) {
            auto y = block_info.vertices.p00.y + step.y * static_cast<float>(i);
            for (auto j = 0u; j < x_edge_points; ++j) {
                auto x = (j == x_steps) ? block_info.vertices.p10.x
                                        : block_info.vertices.p00.x + step.x * static_cast<float>(j);
                block.push_back(interpolator(glm::vec2(x, y)));
            }
        }
    }

    void interpolated_block_on_last_row(GridQuad block_info, const unsigned int new_points,
                                        std::vector<glm::vec3>& block) {
        const unsigned int x_edge_points = new_points + 1;
        const unsigned int y_edge_points = new_points + 2;
        const unsigned int x_steps = new_points + 1;
        const unsigned int y_steps = new_points + 1;

        auto interpolator = [&block_info](const glm::vec2 interpolation_point) -> glm::vec3 {
            const auto& angles = block_info.vertices;
            const auto& f = block_info.f_in_vertex;
            const float denominator = (angles.p11.x - angles.p00.x) * (angles.p11.y - angles.p00.y);
            const auto term00 = f.p00 * (angles.p11.x - interpolation_point.x) * (angles.p11.y - interpolation_point.y);
            const auto term10 = f.p10 * (interpolation_point.x - angles.p00.x) * (angles.p11.y - interpolation_point.y);
            const auto term01 = f.p01 * (angles.p11.x - interpolation_point.x) * (interpolation_point.y - angles.p00.y);
            const auto term11 = f.p11 * (interpolation_point.x - angles.p00.x) * (interpolation_point.y - angles.p00.y);
            return (term00 + term01 + term10 + term11) / denominator;
        };

        const auto step = block_info.compute_sampling_step(x_steps, y_steps);
        for (auto i = 0u; i < y_edge_points; ++i) {
            auto y = (i == y_steps) ? block_info.vertices.p01.y
                                    : block_info.vertices.p00.y + step.y * static_cast<float>(i);
            for (auto j = 0u; j < x_edge_points; ++j) {
                auto x = block_info.vertices.p00.x + step.x * static_cast<float>(j);
                block.push_back(interpolator(glm::vec2(x, y)));
            }
        }
    }

    void interpolated_block(GridQuad block_info, const unsigned int new_points,
                            std::vector<glm::vec3>& block) {
        const unsigned int edge_points = new_points + 1;
        const unsigned int steps = new_points + 1;

        auto interpolator = [&block_info](const glm::vec2 interpolation_point) -> glm::vec3 {
            const auto& angles = block_info.vertices;
            const auto& f = block_info.f_in_vertex;
            const float denominator = (angles.p11.x - angles.p00.x) * (angles.p11.y - angles.p00.y);
            const auto term00 = f.p00 * (angles.p11.x - interpolation_point.x) * (angles.p11.y - interpolation_point.y);
            const auto term10 = f.p10 * (interpolation_point.x - angles.p00.x) * (angles.p11.y - interpolation_point.y);
            const auto term01 = f.p01 * (angles.p11.x - interpolation_point.x) * (interpolation_point.y - angles.p00.y);
            const auto term11 = f.p11 * (interpolation_point.x - angles.p00.x) * (interpolation_point.y - angles.p00.y);
            return (term00 + term01 + term10 + term11) / denominator;
        };

        const auto step = block_info.compute_sampling_step(steps, steps);
        for (auto i = 0u; i < edge_points; ++i) {
            auto y = block_info.vertices.p00.y + step.y * static_cast<float>(i);
            for (auto j = 0u; j < edge_points; ++j) {
                auto x = block_info.vertices.p00.x + step.x * static_cast<float>(j);
                block.push_back(interpolator(glm::vec2(x, y)));
            }
        }
    }

    vec3_grid compose_interpolated_blocks_as_grid(GridDimension original_dimensions, unsigned new_points_per_edge, vec3_grid&& blocks) {
        vec3_grid result;
        const auto sub_block_rows_number = (new_points_per_edge + 1);
        const auto resulting_rows_number = (original_dimensions.height - 1) * sub_block_rows_number + 1;
        const auto resulting_columns_number = (original_dimensions.width - 1) * sub_block_rows_number + 1;
        result.reserve(resulting_rows_number);
        for (auto i = 0u; i < original_dimensions.height - 1; ++i) {
            for (auto k = 0u; k < new_points_per_edge + 1; ++k) {
                std::vector<glm::vec3> row;
                row.reserve(resulting_columns_number);
                for (auto j = 0u; j < original_dimensions.width - 1; ++j) {
                    const bool is_last_column = (j == original_dimensions.width - 2);
                    const auto block_index = i * (original_dimensions.width - 1) + j;
                    if (is_last_column) {
                        std::move(blocks[block_index].begin() + static_cast<int>((sub_block_rows_number + 1) * k),
                                  blocks[block_index].begin() + static_cast<int>((sub_block_rows_number + 1) * (k + 1)),
                                  std::back_inserter(row));
                    } else {
                        std::move(blocks[block_index].begin() + static_cast<int>(sub_block_rows_number * k),
                                  blocks[block_index].begin() + static_cast<int>(sub_block_rows_number * (k + 1)),
                                  std::back_inserter(row));
                    }
                }
                result.push_back(row);
            }
        }

        // Handling last line
        std::vector<glm::vec3> row;
        row.reserve(resulting_columns_number);
        for (auto j = 0u; j < original_dimensions.width - 1; ++j) {
            const bool is_last_column = (j == original_dimensions.width - 2);
            const auto block_index = (original_dimensions.height - 2) * (original_dimensions.width - 1) + j;
            if (is_last_column) {
                std::move(blocks[block_index].begin() +
                          static_cast<int>((sub_block_rows_number + 1) * sub_block_rows_number ),
                          blocks[block_index].begin() +
                          static_cast<int>((sub_block_rows_number + 1) * (sub_block_rows_number + 1)),
                          std::back_inserter(row));
            } else {
                std::move(blocks[block_index].begin() + static_cast<int>(sub_block_rows_number * sub_block_rows_number),
                          blocks[block_index].begin() +
                          static_cast<int>(sub_block_rows_number * (sub_block_rows_number + 1)),
                          std::back_inserter(row));
            }
        }
        result.push_back(row);

        return result;
    }
}