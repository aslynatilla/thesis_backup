#ifndef IES_MESH_TYPE_SPECIFIC_ADAPTER_H
#define IES_MESH_TYPE_SPECIFIC_ADAPTER_H

#include "../ies_document.h"

#include <glm/glm.hpp>

namespace ies::adapter{
    using vec3_grid = std::vector<std::vector<glm::vec3>>;

    void flip_left(vec3_grid & point_grid);
    void flip_back(vec3_grid & point_grid);
}

namespace ies::adapter::type_a_b {
    void transform_grid(const Photometric_Angles& light_data, vec3_grid& point_grid);
}

namespace ies::adapter::type_c {
    void transform_grid(const Photometric_Angles& light_data, vec3_grid& point_grid);
}


#endif //IES_MESH_TYPE_SPECIFIC_ADAPTER_H
