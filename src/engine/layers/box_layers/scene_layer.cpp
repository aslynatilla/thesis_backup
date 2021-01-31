#include "scene_layer.h"

void engine::SceneLayer::on_attach() {
    {
        Assimp::Importer test_importer;
        const aiScene* cornell_box_scene = test_importer.ReadFile("resources/cornell_box_multimaterial.obj", 0);
        if(cornell_box_scene != nullptr && cornell_box_scene->HasMeshes()){
            fmt::print("[SCENE LAYER] Found {} meshes.\n", cornell_box_scene->mNumMeshes);

            if(cornell_box_scene->mNumMaterials > 0){
                fmt::print("[SCENE LAYER] Found {} materials too.\n", cornell_box_scene->mNumMaterials);
            }

            for(unsigned int i = 0; i < cornell_box_scene->mNumMeshes; ++i){
                fmt::print("[SCENE LAYER] Mesh #{} has both positions and normals.\n", i+1,
                           cornell_box_scene->mMeshes[i]->HasPositions() && cornell_box_scene->mMeshes[i]->HasNormals());
            }
        }
    }
}

void engine::SceneLayer::on_detach() {}

void engine::SceneLayer::on_event([[maybe_unused]] engine::Event& event) {
}
