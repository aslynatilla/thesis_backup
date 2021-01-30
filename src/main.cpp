#include "application.h"

#pragma warning(disable : 4061)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#pragma warning(error : 4061)

int main(){
    Assimp::Importer test_importer;
    const aiScene* cornell_box_scene = test_importer.ReadFile("resources/cornell_box_multimaterial.obj", 0);
    if(cornell_box_scene != nullptr && cornell_box_scene->HasMeshes()){
        fmt::print("[MAIN] Found {} meshes.\n", cornell_box_scene->mNumMeshes);

        if(cornell_box_scene->mNumMaterials > 0){
            fmt::print("[MAIN] Found {} materials too.\n", cornell_box_scene->mNumMaterials);
        }
    }

    Application built_application;
    built_application.run();
    return 0;
}