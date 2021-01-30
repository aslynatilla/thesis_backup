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

    glm::vec3 polar_coordinates_to_unit_vector(const float vertical_angle, const float horizontal_angle);

    std::vector<std::vector<glm::vec3>> directions_from_angles(const std::vector<float>& vertical_angles,
                                                               const std::vector<float>& horizontal_angles);

    std::vector<std::vector<glm::vec3>> points_from_directions(std::vector<float>::const_iterator first_scale_factor,
                                                               std::vector<std::vector<glm::vec3>>&& unit_positions);

    std::array<unsigned int, 3> top_left_quad_triangle(const unsigned row,
                                                       const unsigned col,
                                                       const unsigned columns);

    std::array<unsigned int, 3> bottom_right_quad_triangle(const unsigned row,
                                                           const unsigned col,
                                                           const unsigned columns);

    std::vector<unsigned int> triangle_indices_from_grid(const unsigned int rows, const unsigned int columns);

    std::optional<glm::vec3> is_valid_grid_position(const unsigned int row, const unsigned int col,
                                                    const std::vector<std::vector<glm::vec3>>& point_grid);

    glm::vec3 triangle_normal(const glm::vec3& start_vertex,
                              const glm::vec3& first_end_vertex,
                              const glm::vec3& second_end_vertex);

    glm::vec3 normal_at_vertex(const unsigned int row, const unsigned int col,
                               const std::vector<std::vector<glm::vec3>>& point_grid);

    std::vector<glm::vec3> calculate_normals(const std::vector<std::vector<glm::vec3>>& point_grid);

}


#endif //IES_MESH_ADAPTER_H
