#include "mesh_interpolation.h"

namespace ies::adapter {
    vec3_grid interpolate_grid(const vec2_grid& angles, vec3_grid&& points, uint16_t new_points_per_edge) {
        using namespace impl_details;
        const auto dimensions = GridDimension::from(points);

        //  Flattened blocks are the sub-grids, the areas we build an interpolated function over
        vec3_grid grid_of_blocks;
        grid_of_blocks.reserve((dimensions.height - 1) * (dimensions.width - 1));
        for (auto row = 0u; row < dimensions.height - 1; ++row) {
            for (auto col = 0u; col < dimensions.width - 1; ++col) {
                const bool is_last_row = (row == dimensions.height - 2);
                const bool is_last_column = (col == dimensions.width - 2);
                auto block = reserve_block_space(is_last_row, is_last_column, new_points_per_edge);

                BlockDescriptor block_description{
                        .block_corners = {.p00 = angles[row][col],
                                .p01 = angles[row + 1][col],
                                .p10 = angles[row][col + 1],
                                .p11 = angles[row + 1][col + 1]},
                        .f_in_corners = {.p00 = points[row][col],
                                .p01 = points[row + 1][col],
                                .p10 = points[row][col + 1],
                                .p11 = points[row + 1][col + 1]},

                };

                if (is_last_column && is_last_row) {
                    last_interpolated_block(block_description, new_points_per_edge, block);
                } else if (is_last_column) {
                    interpolated_block_on_last_col(block_description, new_points_per_edge, block);
                } else if (is_last_row) {
                    interpolated_block_on_last_row(block_description, new_points_per_edge, block);
                } else {
                    interpolated_block(block_description, new_points_per_edge, block);
                }
                grid_of_blocks.push_back(block);
            }
        }

        return compose_interpolated_grid(dimensions, new_points_per_edge, std::move(grid_of_blocks));
    }
};

namespace ies::adapter::impl_details {
    GridDimension GridDimension::from(const vec3_grid& target_grid) {
        return GridDimension{.width = target_grid[0].size(),
                .height = target_grid.size()};
    }

    glm::vec2
    BlockDescriptor::compute_interpolation_step(unsigned short points_per_x_edge,
                                                unsigned short points_per_y_edge) const {
        const auto x_edge_points = static_cast<float>(points_per_x_edge);
        const auto y_edge_points = static_cast<float>(points_per_y_edge);
        return glm::vec2((block_corners.p10.x - block_corners.p00.x) / x_edge_points,
                         (block_corners.p01.y - block_corners.p00.y) / y_edge_points);
    }

    std::vector<glm::vec3>
    reserve_block_space(const bool is_last_row_block, const bool is_last_column_block, uint16_t new_points_per_edge) {
        std::vector<glm::vec3> block;
        if (is_last_column_block && is_last_row_block) {
            block.reserve((new_points_per_edge + 2) * (new_points_per_edge + 2));
        } else if (is_last_column_block || is_last_row_block) {
            block.reserve((new_points_per_edge + 2) * (new_points_per_edge + 1));
        } else {
            block.reserve((new_points_per_edge + 1) * (new_points_per_edge + 1));
        }
        return block;
    }

    void last_interpolated_block(BlockDescriptor block_info, unsigned short new_points,
                                 std::vector<glm::vec3>& block) {
        const unsigned short x_edge_points = new_points + 2;
        const unsigned short y_edge_points = new_points + 2;

        auto interpolator = [&block_info](const glm::vec2 interpolation_point) -> glm::vec3 {
            const auto& angles = block_info.block_corners;
            const auto& f = block_info.f_in_corners;
            const float denominator = (angles.p11.x - angles.p00.x) * (angles.p11.y - angles.p00.y);
            const auto term00 = f.p00 * (angles.p11.x - interpolation_point.x) * (angles.p11.y - interpolation_point.y);
            const auto term10 = f.p10 * (interpolation_point.x - angles.p00.x) * (angles.p11.y - interpolation_point.y);
            const auto term01 = f.p01 * (angles.p11.x - interpolation_point.x) * (interpolation_point.y - angles.p00.y);
            const auto term11 = f.p11 * (interpolation_point.x - angles.p00.x) * (interpolation_point.y - angles.p00.y);
            return (term00 + term01 + term10 + term11) / denominator;
        };

        const auto step = block_info.compute_interpolation_step(x_edge_points, y_edge_points);
        for (auto i = 0; i < y_edge_points; ++i) {
            auto y = (i == y_edge_points - 1) ? block_info.block_corners.p11.y
                                              : block_info.block_corners.p00.y + step.y * i;
            for (auto j = 0; j < x_edge_points; ++j) {
                auto x = (j == x_edge_points - 1) ? block_info.block_corners.p11.x
                                                  : block_info.block_corners.p00.x + step.x * j;
                block.push_back(interpolator(glm::vec2(x, y)));
            }
        }
    }

    void interpolated_block_on_last_col(BlockDescriptor block_info, unsigned short new_points,
                                        std::vector<glm::vec3>& block) {
        const unsigned short x_edge_points = new_points + 2;
        const unsigned short y_edge_points = new_points + 1;

        auto interpolator = [&block_info](const glm::vec2 interpolation_point) -> glm::vec3 {
            const auto& angles = block_info.block_corners;
            const auto& f = block_info.f_in_corners;
            const float denominator = (angles.p11.x - angles.p00.x) * (angles.p11.y - angles.p00.y);
            const auto term00 = f.p00 * (angles.p11.x - interpolation_point.x) * (angles.p11.y - interpolation_point.y);
            const auto term10 = f.p10 * (interpolation_point.x - angles.p00.x) * (angles.p11.y - interpolation_point.y);
            const auto term01 = f.p01 * (angles.p11.x - interpolation_point.x) * (interpolation_point.y - angles.p00.y);
            const auto term11 = f.p11 * (interpolation_point.x - angles.p00.x) * (interpolation_point.y - angles.p00.y);
            return (term00 + term01 + term10 + term11) / denominator;
        };

        const auto step = block_info.compute_interpolation_step(x_edge_points, y_edge_points);
        for (auto i = 0; i < y_edge_points; ++i) {
            auto y = block_info.block_corners.p00.y + step.y * i;
            for (auto j = 0; j < x_edge_points; ++j) {
                auto x = (j == x_edge_points - 1) ? block_info.block_corners.p11.x
                                                  : block_info.block_corners.p00.x + step.x * j;
                block.push_back(interpolator(glm::vec2(x, y)));
            }
        }
    }

    void interpolated_block_on_last_row(BlockDescriptor block_info, unsigned short new_points,
                                        std::vector<glm::vec3>& block) {
        const unsigned short x_edge_points = new_points + 1;
        const unsigned short y_edge_points = new_points + 2;

        auto interpolator = [&block_info](const glm::vec2 interpolation_point) -> glm::vec3 {
            const auto& angles = block_info.block_corners;
            const auto& f = block_info.f_in_corners;
            const float denominator = (angles.p11.x - angles.p00.x) * (angles.p11.y - angles.p00.y);
            const auto term00 = f.p00 * (angles.p11.x - interpolation_point.x) * (angles.p11.y - interpolation_point.y);
            const auto term10 = f.p10 * (interpolation_point.x - angles.p00.x) * (angles.p11.y - interpolation_point.y);
            const auto term01 = f.p01 * (angles.p11.x - interpolation_point.x) * (interpolation_point.y - angles.p00.y);
            const auto term11 = f.p11 * (interpolation_point.x - angles.p00.x) * (interpolation_point.y - angles.p00.y);
            return (term00 + term01 + term10 + term11) / denominator;
        };

        const auto step = block_info.compute_interpolation_step(x_edge_points, y_edge_points);
        for (auto i = 0; i < y_edge_points; ++i) {
            auto y = (i == y_edge_points - 1) ? block_info.block_corners.p11.y
                                              : block_info.block_corners.p00.y + step.y * i;
            for (auto j = 0; j < x_edge_points; ++j) {
                auto x = block_info.block_corners.p00.x + step.x * j;
                block.push_back(interpolator(glm::vec2(x, y)));
            }
        }
    }

    void interpolated_block(BlockDescriptor block_info, unsigned short new_points,
                            std::vector<glm::vec3>& block) {
        const unsigned short edge_points = new_points + 1;

        auto interpolator = [&block_info](const glm::vec2 interpolation_point) -> glm::vec3 {
            const auto& angles = block_info.block_corners;
            const auto& f = block_info.f_in_corners;
            const float denominator = (angles.p11.x - angles.p00.x) * (angles.p11.y - angles.p00.y);
            const auto term00 = f.p00 * (angles.p11.x - interpolation_point.x) * (angles.p11.y - interpolation_point.y);
            const auto term10 = f.p10 * (interpolation_point.x - angles.p00.x) * (angles.p11.y - interpolation_point.y);
            const auto term01 = f.p01 * (angles.p11.x - interpolation_point.x) * (interpolation_point.y - angles.p00.y);
            const auto term11 = f.p11 * (interpolation_point.x - angles.p00.x) * (interpolation_point.y - angles.p00.y);
            return (term00 + term01 + term10 + term11) / denominator;
        };

        const auto step = block_info.compute_interpolation_step(edge_points, edge_points);
        for (auto i = 0; i < edge_points; ++i) {
            auto y = block_info.block_corners.p00.y + step.y * i;
            for (auto j = 0; j < edge_points; ++j) {
                auto x = block_info.block_corners.p00.x + step.x * j;
                block.push_back(interpolator(glm::vec2(x, y)));
            }
        }
    }

    vec3_grid compose_interpolated_grid(GridDimension dimension, unsigned new_points_per_edge, vec3_grid&& blocks) {
        vec3_grid result;
        const auto sub_block_rows_number = (new_points_per_edge + 1);
        const auto resulting_rows_number = (dimension.height - 1) * sub_block_rows_number + 1;
        const auto resulting_columns_number = (dimension.width - 1) * sub_block_rows_number + 1;
        result.reserve(resulting_rows_number);
        for (auto i = 0u; i < dimension.height - 1; ++i) {
            for (auto k = 0u; k < new_points_per_edge + 1; ++k) {
                std::vector<glm::vec3> row;
                row.reserve(resulting_columns_number);
                for (auto j = 0u; j < dimension.width - 1; ++j) {
                    const bool is_last_column = (j == dimension.width - 2);
                    const auto block_index = i * (dimension.width - 1) + j;
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
        for (auto j = 0u; j < dimension.width - 1; ++j) {
            const bool is_last_column = (j == dimension.width - 2);
            const auto block_index = (dimension.height - 2) * (dimension.width - 1) + j;
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