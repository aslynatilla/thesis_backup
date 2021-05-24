#include "ies_mesh_type_specific_adapter.h"

namespace ies::adapter{
    //  Flip left, opposed to right as the direction of the x-axis
    //  Flipped elements are put before non-flipped ones
    void flip_left(vec3_grid & point_grid){
        const glm::vec3 flip_value { -1.0f, 1.0f, 1.0f };

        std::vector<std::vector<glm::vec3>> new_grid;
        new_grid.reserve(point_grid.size());

        for (const auto& row : point_grid) {
            const auto             row_size = row.size();
            std::vector<glm::vec3> new_row;
            new_row.reserve(row_size * 2);
            std::transform(std::begin(row), std::end(row), std::back_inserter(new_row),
                           [&flip_value](auto x) { return x * flip_value; });
            std::reverse(std::begin(new_row), std::begin(new_row) + row_size);
            std::copy(std::begin(row), std::end(row), std::back_inserter(new_row));
            new_grid.emplace_back(std::move(new_row));
        }

        point_grid.clear();
        std::move(std::begin(new_grid), std::end(new_grid), std::back_inserter(point_grid));
    }

    //  Flip back, opposed to forward as the direction of the z-axis
    //  Flipped elements are put before non-flipped ones
    void flip_back(vec3_grid & point_grid){
        const glm::vec3 flip_value { 1.0f, 1.0f, -1.0f };
        const auto                          rows_number = point_grid.size();

        std::vector<std::vector<glm::vec3>> new_grid;
        new_grid.reserve(rows_number * 2);

        for (auto& row : point_grid) {
            std::vector<glm::vec3> new_row;
            new_row.reserve(row.size());

            std::transform(std::begin(row), std::end(row), std::back_inserter(new_row),
                           [&flip_value](auto x) { return x * flip_value; });
            new_grid.emplace_back(std::move(new_row));
        }

        std::reverse(std::begin(new_grid), std::begin(new_grid) + rows_number);
        std::copy(std::begin(point_grid), std::end(point_grid), std::back_inserter(new_grid));

        point_grid.clear();
        std::move(std::begin(new_grid), std::end(new_grid), std::back_inserter(point_grid));
    }
}

namespace ies::adapter::type_a_b {
    void transform_grid(const Photometric_Angles& light_data, vec3_grid & point_grid){
        if(light_data.horizontal_angles.front() == 0.0f && light_data.horizontal_angles.back() == 90.0f) {
            flip_left(point_grid);
            flip_back(point_grid);
        }

        if(light_data.horizontal_angles.front() == -90.0f && light_data.horizontal_angles.back() == 90.0f) {
            flip_left(point_grid);
        }
    }
}

namespace ies::adapter::type_c {
    void transform_grid(const Photometric_Angles& light_data, vec3_grid & point_grid){
        if (light_data.horizontal_angles.size() == 1 && light_data.horizontal_angles.front() == 0.0f) {
            // The luminaire is symmetric in all [vertical] photometric planes
            // It should be represented as some sort of cylinder
            // How to represent it here, though?
        } else {
            if (light_data.horizontal_angles.front() == 0.0f) {
                if (light_data.horizontal_angles.back() == 90.0f) {
                    // The luminaire is symmetric in each quadrant; flip it around the origin
                    flip_left(point_grid);
                    flip_back(point_grid);
                } else if (light_data.horizontal_angles.back() == 180.0f) {
                    // The luminaire is bilaterally symmetric about the 0-180 photometric plane
                    flip_back(point_grid);
                } else {
                    // The luminaire exhibits no lateral symmetry, so fallback to naive (no-op)
                }
            } else if (light_data.horizontal_angles.front() == 90.0f && light_data.horizontal_angles.back() == 270.0f) {
                // The luminaire is bilaterally symmetric about the 90-270 photometric plane
                flip_left(point_grid);
            }
        }
    }
}