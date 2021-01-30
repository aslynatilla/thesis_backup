#ifndef IES_PARSER_IES_MESH_TYPE_SPECIFIC_ADAPTER_H
#define IES_PARSER_IES_MESH_TYPE_SPECIFIC_ADAPTER_H

#include "../ies_document.h"

#include <glm/glm.hpp>

namespace ies::adapter{
    void flip_left(std::vector<std::vector<glm::vec3>>& point_grid);
    void flip_back(std::vector<std::vector<glm::vec3>>& point_grid);
}

namespace ies::adapter::type_a_b {
    void transform_grid(const Photometric_Angles& light_data, std::vector<std::vector<glm::vec3>>& point_grid);
}

namespace ies::adapter::type_c {
    void transform_grid(const Photometric_Angles& light_data, std::vector<std::vector<glm::vec3>>& point_grid);
}


#endif //IES_PARSER_IES_MESH_TYPE_SPECIFIC_ADAPTER_H
