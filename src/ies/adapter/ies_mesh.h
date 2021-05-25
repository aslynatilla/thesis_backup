#ifndef IES_MESH_H
#define IES_MESH_H

#include "ies_mesh_adapter.h"
#include "ies_mesh_type_specific_adapter.h"
#include "mesh_interpolation.h"

#include "glm/glm.hpp"

#include <utility>

namespace ies::adapter {

    class IES_Mesh {
    public:
        explicit IES_Mesh(const IES_Document& source_document);

        IES_Mesh() = default;

        ~IES_Mesh() = default;

        IES_Mesh(const IES_Mesh& other) = delete;

        IES_Mesh(IES_Mesh&& other) = default;

        IES_Mesh& operator=(const IES_Mesh& other) = delete;

        IES_Mesh& operator=(IES_Mesh&& other) = default;

        IES_Mesh& convert_from(const IES_Document& source_document);

        static IES_Mesh interpolate_from(const IES_Document& document,
                                         uint16_t interpolated_points_per_edge);

        static IES_Mesh debug_mesh();
        explicit IES_Mesh(bool debuggable);

        [[nodiscard]] std::vector<float> get_vertices() const;

        [[nodiscard]] std::vector<unsigned int> get_indices() const noexcept;

    private:
        IES_Mesh(const IES_Document& document,
                 uint16_t interpolated_points_per_edge);

        void compute_mesh_from(const Photometric_Angles& light_data,
                               const Photometric_Type& light_type,
                               vec3_grid&& points);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

    };

}

#endif //IES_MESH_H
