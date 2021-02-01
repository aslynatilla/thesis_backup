#include "scene_object.h"

namespace engine{
    SceneObject::SceneObject() :
        vao{std::make_unique<VertexArray>()},
        transform{glm::identity<glm::mat4>()},
        material{}
        {}

    void SceneObject::set_transform_matrix(aiMatrix4x4 model_matrix){
        const auto& m = model_matrix.Transpose();

        transform = glm::mat4(m.a1, m.a2, m.a3, m.a4,
                              m.b1, m.b2, m.b3, m.b4,
                              m.c1, m.c2, m.c3, m.c4,
                              m.d1, m.d2, m.d3, m.d4);
    }
}