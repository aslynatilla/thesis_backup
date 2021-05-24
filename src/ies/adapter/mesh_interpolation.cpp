#include "mesh_interpolation.h"

namespace ies::adapter{
    vec3_grid interpolate_grid(const vec2_grid& angles, vec3_grid&& points, uint16_t new_points_per_edge) {
        using namespace impl_details;
        const auto dimensions = GridDimension::from(points);

        //  Flattened blocks are the sub-grids, the areas we build an interpolated function over
        vec3_grid grid_of_blocks;
        grid_of_blocks.reserve((dimensions.height - 1) * (dimensions.width - 1));
        for(auto row = 0u; row < dimensions.height - 1; ++row){
            for(auto col = 0u; col < dimensions.width - 1; ++col ){
                const bool is_last_row = (row == dimensions.height - 2);
                const bool is_last_column = (col == dimensions.width - 2);
                auto block = reserve_block_space(is_last_row, is_last_column, new_points_per_edge);
                auto block_inserter = std::back_inserter(block);
                BlockDescriptor block_description{
                    .block_corners = {.p00 = angles[row][col],
                                      .p01 = angles[row][col+1],
                                      .p10 = angles[row+1][col],
                                      .p11 = angles[row+1][col+1]},
                    .f_in_corners = {.p00 = points[row][col],
                                      .p01 = points[row][col+1],
                                      .p10 = points[row+1][col],
                                      .p11 = points[row+1][col+1]},

                };
                if(is_last_column && is_last_row){
                    last_interpolated_block(block_description, new_points_per_edge, block_inserter);
                } else if(is_last_column){
                    interpolated_block_on_last_col(block_description, new_points_per_edge, block_inserter);
                } else if( is_last_row){
                    interpolated_block_on_last_row(block_description, new_points_per_edge, block_inserter);
                } else {
                    interpolated_block(block_description, new_points_per_edge, block_inserter);
                }
                grid_of_blocks.push_back(block);
            }
        }

        return compose_interpolated_grid(dimensions, new_points_per_edge, std::move(grid_of_blocks));
    }
};

namespace ies::adapter::impl_details{
    GridDimension GridDimension::from(const vec3_grid& target_grid) {
        return GridDimension{.width = target_grid[0].size(),
                             .height = target_grid.size()};
    }

    std::vector<glm::vec3>
    reserve_block_space(const bool is_last_row_block, const bool is_last_column_block, uint16_t new_points_per_edge) {
        std::vector<glm::vec3> block;
        if(is_last_column_block && is_last_row_block){
            block.reserve((new_points_per_edge + 2) * (new_points_per_edge + 2));
        } else if(is_last_column_block || is_last_row_block){
            block.reserve((new_points_per_edge + 2) * (new_points_per_edge + 1));
        } else {
            block.reserve((new_points_per_edge + 1) * (new_points_per_edge + 1));
        }
        return block;
    }

    void last_interpolated_block(BlockDescriptor block_info, unsigned short new_points,
                                 std::back_insert_iterator<std::vector<glm::vec3>> block_inserter) {
        //  Dummy implementation
        const int block_dim = (new_points + 2) * (new_points + 2);
        for(int i = 0; i < block_dim; ++i){
            block_inserter = block_info.f_in_corners.p00;
        }
    }

    void interpolated_block_on_last_col(BlockDescriptor block_info, unsigned short new_points,
                                        std::back_insert_iterator<std::vector<glm::vec3>> block_inserter) {
        //  Dummy implementation
        const int block_dim = (new_points + 2) * (new_points + 1);
        for(int i = 0; i < block_dim; ++i){
            block_inserter = block_info.f_in_corners.p00;
        }
    }

    void interpolated_block_on_last_row(BlockDescriptor block_info, unsigned short new_points,
                                        std::back_insert_iterator<std::vector<glm::vec3>> block_inserter) {
        //  Dummy implementation
        const int block_dim = (new_points + 2) * (new_points + 1);
        for(int i = 0; i < block_dim; ++i){
            block_inserter = block_info.f_in_corners.p00;
        }
    }

    void interpolated_block(BlockDescriptor block_info, unsigned short new_points,
                            std::back_insert_iterator<std::vector<glm::vec3>> block_inserter) {
        //  Dummy implementation
        const int block_dim = (new_points + 1) * (new_points + 1);
        for(int i = 0; i < block_dim; ++i){
            block_inserter = block_info.f_in_corners.p00;
        }
    }

    vec3_grid compose_interpolated_grid([[maybe_unused]] GridDimension dimension, [[maybe_unused]] uint16_t new_points_per_edge, [[maybe_unused]] vec3_grid&& blocks) {
        return ies::adapter::vec3_grid();
    }
}