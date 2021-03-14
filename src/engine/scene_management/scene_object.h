#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include "../rendering/vertex_array.h"
#include "../rendering/material.h"

#pragma warning(disable : 4061)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#pragma warning(error : 4061)

namespace engine{
    class SceneObject{
    public:
        SceneObject();

        void set_transform_matrix(aiMatrix4x4 model_matrix);

        std::unique_ptr<VertexArray> vao;
        glm::mat4 transform;
        glm::mat4 transpose_inverse_transform;
        Material material;
    };
}

#endif //SCENE_OBJECT_H
