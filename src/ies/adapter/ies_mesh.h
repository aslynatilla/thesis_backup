#ifndef IES_MESH_H
#define IES_MESH_H

#include "ies_mesh_adapter.h"
#include "ies_mesh_type_specific_adapter.h"

#include "glm/glm.hpp"

#include <utility>

namespace ies::adapter {

    class IES_Mesh {
    public:
        explicit IES_Mesh();
        explicit IES_Mesh(const IES_Document& source_document);
        IES_Mesh& operator=(const IES_Document& source_document);

        ~IES_Mesh() = default;
        IES_Mesh(const IES_Mesh& other) = delete;
        IES_Mesh(IES_Mesh&& other) = delete;
        IES_Mesh& operator=(const IES_Mesh& other) = delete;
        IES_Mesh& operator=(IES_Mesh&& other) = delete;

        [[no_discard]] std::vector<float> get_vertices() const;
        [[no_discard]] std::vector<unsigned int> get_indices() const noexcept;

    private:
        const IES_Document* convertible;

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;
    };

}

#endif //IES_MESH_H
