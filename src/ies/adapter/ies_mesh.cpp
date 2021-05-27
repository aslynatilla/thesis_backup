#include "ies_mesh.h"

namespace ies::adapter {
    IES_Mesh::IES_Mesh(const IES_Document& source_document){
        const auto& light_data = source_document.photometric_description.measured_data;
        const auto& source_type = source_document.photometric_description.data_type;
        auto positions_grid = points_from_directions(std::begin(light_data.candelas_per_angle_pair),
                                                     directions_from_angles(light_data.vertical_angles,
                                                                            light_data.horizontal_angles));

        compute_mesh_from(light_data, source_type, std::move(positions_grid));
    }

    IES_Mesh& IES_Mesh::convert_from(const IES_Document& source_document) {
        indices.clear();
        normals.clear();
        positions.clear();
        const auto& light_data = source_document.photometric_description.measured_data;
        const auto& source_type = source_document.photometric_description.data_type;
        auto positions_grid = points_from_directions(std::begin(light_data.candelas_per_angle_pair),
                                                     directions_from_angles(light_data.vertical_angles,
                                                                            light_data.horizontal_angles));
        compute_mesh_from(light_data, source_type, std::move(positions_grid));
        return *this;
    }

    IES_Mesh IES_Mesh::interpolate_from(const IES_Document& document, const uint16_t interpolated_points_per_edge){
        return IES_Mesh(document, interpolated_points_per_edge);
    }


    std::vector<float> IES_Mesh::get_vertices() const {
        constexpr auto vertex_position_components = 3;
        constexpr auto vertex_normal_components = 3;
        const auto number_of_vertices = positions.size();

        std::vector<float> vertices;
        vertices.reserve((vertex_position_components + vertex_normal_components) * number_of_vertices);

        auto vertices_back_inserter = std::back_inserter(vertices);
        auto positions_it = std::begin(positions);
        auto normals_it = std::begin(normals);
        while (positions_it != std::end(positions) && normals_it != std::end(normals)) {
            vertices_back_inserter = positions_it->x;
            vertices_back_inserter = positions_it->y;
            vertices_back_inserter = positions_it->z;
            vertices_back_inserter = normals_it->x;
            vertices_back_inserter = normals_it->y;
            vertices_back_inserter = normals_it->z;
            ++positions_it;
            ++normals_it;
        }
        return vertices;
    }

    std::vector<unsigned int> IES_Mesh::get_indices() const noexcept {
        return std::vector(indices);
    }

    IES_Mesh::IES_Mesh(const IES_Document& document, const unsigned int interpolated_points_per_edge) {
        const auto& light_data = document.photometric_description.measured_data;
        const auto& source_type = document.photometric_description.data_type;
        auto positions_grid = points_from_directions(std::begin(light_data.candelas_per_angle_pair),
                                                     directions_from_angles(light_data.vertical_angles,
                                                                            light_data.horizontal_angles));

                                                        //Rows                      //Columns
        const auto angle_couples = cartesian_product(light_data.horizontal_angles, light_data.vertical_angles);
        positions_grid = interpolate_grid(angle_couples, std::move(positions_grid), interpolated_points_per_edge);
        compute_mesh_from(light_data, source_type, std::move(positions_grid));
    }

    void
    IES_Mesh::compute_mesh_from(const Photometric_Angles& light_data, const Photometric_Type& light_type,
                                vec3_grid&& points) {
        if(light_type == Photometric_Type::Type_A || light_type == Photometric_Type::Type_B) {
            type_a_b::transform_grid(light_data, points);
        } else if(light_type == Photometric_Type::Type_C) {
            type_c::transform_grid(light_data, points);
        }

        indices = triangle_indices_from_grid(points.size(), points[0].size());

        normals = calculate_normals(points);

        positions.reserve(normals.size());
        for(auto& row : points){
            std::move(std::begin(row), std::end(row), std::back_inserter(positions));
        }
    }
}
