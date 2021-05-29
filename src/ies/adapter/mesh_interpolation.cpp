#include "mesh_interpolation.h"

namespace ies::adapter {
    vec3_grid
    interpolate_grid(const vec2_grid& domain_points, vec3_grid&& codomain_points, unsigned int new_points_per_edge) {
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
                auto block = create_block_and_reserve(new_points_per_edge, is_last_row, is_last_column);

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

    glm::vec3 GridQuad::bilinear_interpolation_in(const glm::vec2 point) const {
        const auto& v = vertices;
        const auto& f = f_in_vertex;
        const float denominator = (v.p11.x - v.p00.x) * (v.p11.y - v.p00.y);
        const auto term00 = f.p00 * (v.p11.x - point.x) * (v.p11.y - point.y);
        const auto term10 = f.p10 * (point.x - v.p00.x) * (v.p11.y - point.y);
        const auto term01 = f.p01 * (v.p11.x - point.x) * (point.y - v.p00.y);
        const auto term11 = f.p11 * (point.x - v.p00.x) * (point.y - v.p00.y);
        return (term00 + term01 + term10 + term11) / denominator;
    }

    //   TODO: this might be handled differently if using a struct to represent the block better;
    //  semantically, these should be two different methods, maybe chained...
    //  consider composition over inheritance and a struct containing these vectors
    std::vector<glm::vec3>
    create_block_and_reserve(const unsigned int new_points_per_edge,
                             const bool is_last_row_block,
                             const bool is_last_column_block) {
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

    std::vector<glm::vec2> flat_cartesian_product(const std::vector<float>& first_values,
                                                  const std::vector<float>& second_values) {
        std::vector<glm::vec2> products;
        products.reserve(second_values.size() * first_values.size());
        for (const auto y : second_values) {
            for (const auto x : first_values) {
                products.emplace_back(x, y);
            }
        }
        return products;
    }

    float compute_sampling_step(const unsigned int number_of_steps,
                                const FloatRange range_to_sample) {
        const auto steps_as_float = static_cast<float>(number_of_steps);
        return (range_to_sample.end - range_to_sample.start) / steps_as_float;
    }

    std::vector<float>
    compute_range(unsigned int samples_in_range, FloatRange range_endpoints, bool includes_range_end) {
        std::vector<float> values;
        float step = includes_range_end ? compute_sampling_step(samples_in_range - 1, range_endpoints)
                                        : compute_sampling_step(samples_in_range, range_endpoints);
        for (auto i = 0u; i < samples_in_range; ++i) {
            if (includes_range_end && i == samples_in_range - 1) {
                values.push_back(range_endpoints.end);
            } else {
                values.push_back(range_endpoints.start + step * static_cast<float>(i));
            }
        }
        return values;
    }

    void interpolated_block_last(GridQuad block_info, const unsigned int new_points,
                                 std::vector<glm::vec3>& block) {
        const unsigned int x_edge_points = new_points + 2;
        const unsigned int y_edge_points = new_points + 2;
        const FloatRange x_range{block_info.vertices.p00.x, block_info.vertices.p10.x};
        const FloatRange y_range{block_info.vertices.p00.y, block_info.vertices.p01.y};
        const std::vector<float> xs = compute_range(x_edge_points, x_range, true);
        const std::vector<float> ys = compute_range(y_edge_points, y_range, true);
        const auto points = flat_cartesian_product(xs, ys);
        std::transform(std::begin(points), std::end(points), std::back_inserter(block),
                       [&block_info](const auto p) { return block_info.bilinear_interpolation_in(p); });
    }


    void interpolated_block_on_last_col(GridQuad block_info, const unsigned int new_points,
                                        std::vector<glm::vec3>& block) {
        const unsigned int x_edge_points = new_points + 2;
        const unsigned int y_edge_points = new_points + 1;
        const FloatRange x_range{block_info.vertices.p00.x, block_info.vertices.p10.x};
        const FloatRange y_range{block_info.vertices.p00.y, block_info.vertices.p01.y};
        const std::vector<float> xs = compute_range(x_edge_points, x_range, true);
        const std::vector<float> ys = compute_range(y_edge_points, y_range, false);
        const auto points = flat_cartesian_product(xs, ys);
        std::transform(std::begin(points), std::end(points), std::back_inserter(block),
                       [&block_info](const auto p) { return block_info.bilinear_interpolation_in(p); });
    }

    void interpolated_block_on_last_row(GridQuad block_info, const unsigned int new_points,
                                        std::vector<glm::vec3>& block) {
        const unsigned int x_edge_points = new_points + 1;
        const unsigned int y_edge_points = new_points + 2;
        const FloatRange x_range{block_info.vertices.p00.x, block_info.vertices.p10.x};
        const FloatRange y_range{block_info.vertices.p00.y, block_info.vertices.p01.y};
        const std::vector<float> xs = compute_range(x_edge_points, x_range, false);
        const std::vector<float> ys = compute_range(y_edge_points, y_range, true);
        const auto points = flat_cartesian_product(xs, ys);
        std::transform(std::begin(points), std::end(points), std::back_inserter(block),
                       [&block_info](const auto p) { return block_info.bilinear_interpolation_in(p); });
    }

    void interpolated_block(GridQuad block_info, const unsigned int new_points,
                            std::vector<glm::vec3>& block) {
        const unsigned int edge_points = new_points + 1;
        const FloatRange x_range{block_info.vertices.p00.x, block_info.vertices.p10.x};
        const FloatRange y_range{block_info.vertices.p00.y, block_info.vertices.p01.y};
        const std::vector<float> xs = compute_range(edge_points, x_range, false);
        const std::vector<float> ys = compute_range(edge_points, y_range, false);
        const auto points = flat_cartesian_product(xs, ys);
        std::transform(std::begin(points), std::end(points), std::back_inserter(block),
                       [&block_info](const auto p) { return block_info.bilinear_interpolation_in(p); });
    }

    vec3_grid compose_interpolated_blocks_as_grid(GridDimension original_dimensions, unsigned new_points_per_edge,
                                                  vec3_grid&& blocks) {
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