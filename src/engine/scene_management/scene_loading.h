#ifndef SCENE_LOADING_H
#define SCENE_LOADING_H

#include "scene_object.h"
#include "../rendering/opengl3_texture.h"
#include "../../utility/file_reader.h"

#include "stb_image.h"

namespace engine::scenes{
    using TextureResource = std::unique_ptr<OpenGL3_Texture2D>;

    struct SceneData{
        std::vector<SceneObject> objects;
        std::vector<TextureResource> textures;
    };

    class SceneLoader{
    public:
        static SceneData load_scene_from(const std::filesystem::path& to_scene, unsigned int postprocessing_flags);
    private:
        SceneLoader(std::filesystem::path scene_path, unsigned int flags);
        SceneData process_scene();
        void process_scene_node(const aiScene* scene,
                                const aiNode* node,
                                const aiMatrix4x4& parent_transform);
        SceneObject process_mesh(const aiScene* source_scene,
                                 const aiMesh* mesh,
                                 const aiMatrix4x4& mesh_transform);

        std::filesystem::path path_to_scene;
        unsigned int assimp_postprocessing_flags;
        std::vector<SceneObject> scene_objects;
        std::vector<TextureResource> scene_textures;
    };

    std::vector<float> extract_vertex_data(int vertices_number, aiVector3D* positions, aiVector3D* normals);
    std::vector<float> extract_vertex_data(int vertices_number, aiVector3D* positions, aiVector3D* normals, aiVector3D* uv_coords);
    std::vector<float> get_vertex_data(const aiMesh* source);
    std::vector<unsigned int> get_indices(const aiMesh* source);
    std::vector<unsigned char> load_texture(std::string_view texture_path);
}
#endif //SCENE_LOADING_H
