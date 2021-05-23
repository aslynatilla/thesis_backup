#include "ies_mesh_adapter.h"

namespace ies::adapter {

    glm::vec3 polar_coordinates_to_unit_vector(const float vertical_angle, const float horizontal_angle) {
        const auto h = glm::radians(horizontal_angle);
        const auto v = glm::radians(vertical_angle);
        const auto first_unit_vector = glm::normalize(glm::vec3(glm::sin(v), glm::cos(v), 0.0f));
        return glm::vec3(glm::rotate(glm::identity<glm::mat4>(), h, glm::vec3(0.0f, 1.0f, 0.0f))
                         * glm::vec4(first_unit_vector, 1.0f));
    }

    std::vector<std::vector<glm::vec3>>
    directions_from_angles(const std::vector<float>& vertical_angles, const std::vector<float>& horizontal_angles) {
        std::vector<std::vector<glm::vec3>> directions;
        directions.reserve(horizontal_angles.size());

        auto curried_polar_coordinates_to_unit_vector = [](const float h) {
            return [h](const float v) { return polar_coordinates_to_unit_vector(v, h); };
        };

        std::vector<glm::vec3> destination_row;
        destination_row.reserve(vertical_angles.size());
        for (const auto& horizontal_angle : horizontal_angles) {
            std::transform(std::begin(vertical_angles), std::end(vertical_angles),
                           std::back_inserter(destination_row),
                           curried_polar_coordinates_to_unit_vector(horizontal_angle));
            directions.emplace_back(std::move(destination_row));
            destination_row.clear();
        }

        return directions;
    }

    // By contract, given a grid containing N positions, the scale iterator has to be incrementable
    // through std::advance by N, so that this returns a reasonable result.
    std::vector<std::vector<glm::vec3>> points_from_directions(std::vector<float>::const_iterator first_scale_factor,
                                                               std::vector<std::vector<glm::vec3>>&& unit_positions) {
        auto scale_factor_iterator = first_scale_factor;
        for (auto& row : unit_positions) {
            std::transform(std::begin(row), std::end(row), std::begin(row),
                           [&scale_factor_iterator](auto& position) {
                               auto scaled_position = (*scale_factor_iterator) * position;
                               scale_factor_iterator++;
                               return scaled_position;
                           });
        }
        return std::move(unit_positions);
    }

    std::vector<unsigned int> triangle_indices_from_grid(const unsigned int rows, const unsigned int columns) {
        std::vector<unsigned int> indices;
        constexpr auto points_per_triangle = 3u;
        constexpr auto triangles_per_quad = 2u;
        const auto quads = (rows - 1) * (columns - 1);
        indices.reserve(quads * triangles_per_quad * points_per_triangle);

        for (auto row = 0u; row < rows - 1; ++row) {
            for (auto column = 0u; column < columns - 1; ++column) {
                auto t1 = top_left_quad_triangle(row, column, columns);
                auto t2 = bottom_right_quad_triangle(row, column, columns);
                indices.insert(std::end(indices), std::begin(t1), std::end(t1));
                indices.insert(std::end(indices), std::begin(t2), std::end(t2));
            }
        }

        return indices;
    }

    // Note vertices are taken counter-clockwise
    std::array<unsigned, 3>
    top_left_quad_triangle(const unsigned row, const unsigned col, const unsigned columns) {
        const auto passed_in_vertex = row * columns + col;
        const auto vertex_above_on_the_right = (row + 1) * columns + (col + 1);
        const auto vertex_above = (row + 1) * columns + col;
        return std::array<unsigned int, 3>{passed_in_vertex, vertex_above_on_the_right, vertex_above};
    }

    // Note vertices are taken counter-clockwise
    std::array<unsigned, 3>
    bottom_right_quad_triangle(const unsigned row, const unsigned col, const unsigned columns) {
        const auto passed_in_vertex = row * columns + col;
        const auto vertex_on_the_right = row * columns + (col + 1);
        const auto vertex_above_on_the_right = (row + 1) * columns + (col + 1);
        return std::array<unsigned int, 3>{passed_in_vertex, vertex_on_the_right, vertex_above_on_the_right};
    }

    std::optional<glm::vec3> is_valid_grid_position(const unsigned row, const unsigned col,
                                                    const std::vector<std::vector<glm::vec3>>& point_grid) {
        if (row < point_grid.size() && col < point_grid.at(row).size()) {
            return std::optional(point_grid[row][col]);
        } else {
            return std::nullopt;
        }
    }

    glm::vec3 triangle_normal(const glm::vec3& start_vertex, const glm::vec3& first_end_vertex,
                              const glm::vec3& second_end_vertex) {
        constexpr glm::vec3 null_vector(0.0f);

        const auto first_edge = first_end_vertex - start_vertex;
        const auto second_edge = second_end_vertex - start_vertex;
        if (first_edge != second_edge && first_edge != null_vector && second_edge != null_vector) {
            return glm::normalize(glm::cross(first_edge, second_edge));
        } else {
            return null_vector;
        }
    }

    glm::vec3 normal_at_vertex(const unsigned row, const unsigned col,
                               const std::vector<std::vector<glm::vec3>>& point_grid) {
        constexpr glm::vec3 null_vector(0.0f);
        const std::optional passed_in_vertex = is_valid_grid_position(row, col, point_grid);
        const std::optional top_right_vertex = is_valid_grid_position(row + 1, col + 1, point_grid);
        const std::optional right_vertex = is_valid_grid_position(row, col + 1, point_grid);
        const std::optional bottom_vertex = is_valid_grid_position(row - 1, col, point_grid);
        const std::optional bottom_left_vertex = is_valid_grid_position(row - 1, col - 1, point_grid);
        const std::optional left_vertex = is_valid_grid_position(row, col - 1, point_grid);
        const std::optional top_vertex = is_valid_grid_position(row + 1, col, point_grid);

        glm::vec3 normal = null_vector;
        unsigned int considered_triangles = 0;

        if (top_vertex && top_right_vertex) {
            const auto n = triangle_normal(*passed_in_vertex, *top_right_vertex, *top_vertex);
            if (n != null_vector) {
                ++considered_triangles;
                normal += n;
            }
        }

        if (top_vertex && left_vertex) {
            const auto n = triangle_normal(*passed_in_vertex, *top_vertex, *left_vertex);
            if (n != null_vector) {
                ++considered_triangles;
                normal += n;
            }
        }

        if(left_vertex && bottom_left_vertex){
            const auto n = triangle_normal(*passed_in_vertex, *left_vertex, *bottom_left_vertex);
            if(n != null_vector){
                ++considered_triangles;
                normal += n;
            }
        }

        if (bottom_vertex && bottom_left_vertex) {
            const auto n = triangle_normal(*passed_in_vertex, *bottom_left_vertex, *bottom_vertex);
            if (n != null_vector) {
                ++considered_triangles;
                normal += n;
            }
        }
        if (bottom_vertex && right_vertex) {
            const auto n = triangle_normal(*passed_in_vertex, *bottom_vertex, *right_vertex);
            if (n != null_vector) {
                ++considered_triangles;
                normal += n;
            }
        }

        if(right_vertex && top_right_vertex){
            const auto n = triangle_normal(*passed_in_vertex, *right_vertex, *top_right_vertex);
            if(n != null_vector){
                ++considered_triangles;
                normal += n;
            }
        }

        if(considered_triangles != 0){
            normal /= considered_triangles;
        }
        return normal;
    }

    std::vector<glm::vec3> calculate_normals(const vec_grid & point_grid) {
        std::vector<glm::vec3> normals;
        const auto rows = point_grid.size();
        const auto columns = point_grid.at(0).size();
        normals.reserve(rows * columns);

        for (unsigned int row = 0u; row < rows; ++row) {
            for (unsigned int column = 0u; column < columns; ++column) {
                normals.emplace_back(normal_at_vertex(row, column, point_grid));
            }
        }
        return normals;
    }
}