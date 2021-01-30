#include "ies_mesh.h"

namespace ies::adapter {
    IES_Mesh::IES_Mesh() : convertible{nullptr} {}

    IES_Mesh::IES_Mesh(const IES_Document& source_document) : convertible{&source_document} {
        //  Load positions, normals, indices
        const auto& light_data = source_document.photometric_description.measured_data;
        auto positions_grid = points_from_directions(std::begin(light_data.candelas_per_angle_pair),
                                                     directions_from_angles(light_data.vertical_angles,
                                                                            light_data.horizontal_angles));

        const auto& source_type = source_document.photometric_description.data_type;
        if(source_type == Photometric_Type::Type_A || source_type == Photometric_Type::Type_B) {
            type_a_b::transform_grid(light_data, positions_grid);
        } else if(source_type == Photometric_Type::Type_C) {
            type_c::transform_grid(light_data, positions_grid);
        }

        indices = triangle_indices_from_grid(positions_grid.size(), positions_grid[0].size());

        normals = calculate_normals(positions_grid);

        positions.reserve(normals.size());
        for(auto& row : positions_grid){
            std::move(std::begin(row), std::end(row), std::back_inserter(positions));
        }
    }

    IES_Mesh& IES_Mesh::operator=(const IES_Document& source_document) {
        convertible = &source_document;
        indices.clear();
        normals.clear();
        positions.clear();
        const auto& light_data = source_document.photometric_description.measured_data;
        auto positions_grid = points_from_directions(std::begin(light_data.candelas_per_angle_pair),
                                                     directions_from_angles(light_data.vertical_angles,
                                                                            light_data.horizontal_angles));

        const auto& source_type = source_document.photometric_description.data_type;
        if(source_type == Photometric_Type::Type_A || source_type == Photometric_Type::Type_B) {
            type_a_b::transform_grid(light_data, positions_grid);
        } else if(source_type == Photometric_Type::Type_C) {
            type_c::transform_grid(light_data, positions_grid);
        }

        indices = triangle_indices_from_grid(positions_grid.size(), positions_grid[0].size());

        normals = calculate_normals(positions_grid);

        positions.reserve(normals.size());
        for(auto& row : positions_grid){
            std::move(std::begin(row), std::end(row), std::back_inserter(positions));
        }

        return *this;
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


}