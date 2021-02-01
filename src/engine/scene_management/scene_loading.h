#ifndef SCENE_LOADING_H
#define SCENE_LOADING_H

#include "scene_object.h"

namespace engine::scenes{
    void copy_vertices(const aiMesh* source,
                       std::vector<float>& destination);

    void copy_indices(const aiMesh* source,
                      std::vector<unsigned int>& destination);

    SceneObject process_mesh(const aiScene* source_scene,
                             const aiMesh* mesh,
                             const aiMatrix4x4& mesh_transform);
    std::vector<SceneObject> process_scene_node(const aiScene* scene,
                                                const aiNode* node,
                                                const aiMatrix4x4& parent_transform);
    std::vector<SceneObject> load_scene_objects_from(const std::string& path_to_scene,
                                                     const unsigned int assimp_postprocess_flags);
}


#endif //SCENE_LOADING_H
