#ifndef IES_MESH_TYPE_SPECIFIC_ADAPTER_H
#define IES_MESH_TYPE_SPECIFIC_ADAPTER_H

#include "../ies_document.h"

#include <glm/glm.hpp>

namespace ies::adapter{
    using vec_grid = std::vector<std::vector<glm::vec3>>;

    void flip_left(vec_grid & point_grid);
    void flip_back(vec_grid & point_grid);
}

namespace ies::adapter::type_a_b {
    void transform_grid(const Photometric_Angles& light_data, vec_grid& point_grid);
}

namespace ies::adapter::type_c {
    void transform_grid(const Photometric_Angles& light_data, vec_grid& point_grid);
}


#endif //IES_MESH_TYPE_SPECIFIC_ADAPTER_H
