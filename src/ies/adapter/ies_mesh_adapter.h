#ifndef IES_MESH_ADAPTER_H
#define IES_MESH_ADAPTER_H

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <optional>
#include <vector>

namespace ies::adapter {
    using vec2_grid = std::vector<std::vector<glm::vec2>>;
    using vec3_grid = std::vector<std::vector<glm::vec3>>;

    glm::vec3 polar_coordinates_to_unit_vector(float vertical_angle, float horizontal_angle);

    vec3_grid directions_from_angles(const std::vector<float>& vertical_angles,
                                     const std::vector<float>& horizontal_angles);

    vec3_grid points_from_directions(std::vector<float>::const_iterator first_scale_factor,
                                     vec3_grid&& unit_positions);

    std::array<unsigned, 3> top_left_quad_triangle(unsigned row,
                                                       unsigned col,
                                                       unsigned columns);

    std::array<unsigned, 3> bottom_right_quad_triangle(unsigned row,
                                                           unsigned col,
                                                           unsigned columns);

    std::vector<unsigned int> triangle_indices_from_grid(unsigned rows, unsigned columns);

    std::optional<glm::vec3> is_valid_grid_position(unsigned row, unsigned col,
                                                    const vec3_grid& point_grid);

    glm::vec3 triangle_normal(const glm::vec3& start_vertex,
                              const glm::vec3& first_end_vertex,
                              const glm::vec3& second_end_vertex);

    glm::vec3 normal_at_vertex(unsigned row, unsigned col,
                               const vec3_grid& point_grid);

    std::vector<glm::vec3> calculate_normals(const vec3_grid& point_grid);

    vec2_grid cartesian_product(const std::vector<float>& first, const std::vector<float>& second);
}


#endif //IES_MESH_ADAPTER_H
