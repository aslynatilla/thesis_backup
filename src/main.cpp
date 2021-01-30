#include <iostream>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#pragma warning(disable : 4061)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#pragma warning(error : 4061)

int main(){
    Assimp::Importer test_importer;
    const aiScene* cornell_box_scene = test_importer.ReadFile("resources/cornell_box_multimaterial.obj", 0);
    if(cornell_box_scene != nullptr && cornell_box_scene->HasMeshes()){
        std::cout << "Found " << cornell_box_scene->mNumMeshes << " meshes.\n";

        if(cornell_box_scene->mNumMaterials > 0) std::cout << "Found " << cornell_box_scene->mNumMaterials << " materials too.\n";
    }
    return 0;
}