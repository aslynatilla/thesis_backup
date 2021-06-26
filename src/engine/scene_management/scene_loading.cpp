#include "scene_loading.h"

namespace engine::scenes {
    Material convert_assimp_material(const aiMaterial* assimp_material) {
        Material m;
        aiString material_name;
        if (assimp_material->Get(AI_MATKEY_NAME, material_name) != aiReturn_SUCCESS) {
            m.name = Material::default_name;
        } else {
            m.name = std::string(material_name.C_Str());
        }
        if (assimp_material->Get(AI_MATKEY_OPACITY, m.data.opacity) != aiReturn_SUCCESS) {
            m.data.opacity = 1.0f;
        }
        if (assimp_material->Get(AI_MATKEY_SHININESS, m.data.shininess) != aiReturn_SUCCESS) {
            m.data.shininess = 0.0f;
        }
        if (assimp_material->Get(AI_MATKEY_REFRACTI, m.data.refraction_index) != aiReturn_SUCCESS) {
            m.data.refraction_index = 1.0f;
        }
        aiColor3D color;
        if (assimp_material->Get(AI_MATKEY_COLOR_DIFFUSE, color) != aiReturn_SUCCESS) {
            m.data.diffuse_color = glm::vec4(0.0f);
        } else {
            m.data.diffuse_color = glm::vec4(color.r, color.g, color.b, 1.0f);
        }
        if (assimp_material->Get(AI_MATKEY_COLOR_AMBIENT, color) != aiReturn_SUCCESS) {
            m.data.ambient_color = glm::vec4(0.0f);
        } else {
            m.data.ambient_color = glm::vec4(color.r, color.g, color.b, 1.0f);
        }
        if (assimp_material->Get(AI_MATKEY_COLOR_SPECULAR, color) != aiReturn_SUCCESS) {
            m.data.specular_color = glm::vec4(0.0f);
        } else {
            m.data.specular_color = glm::vec4(color.r, color.g, color.b, 1.0f);
        }
        if (assimp_material->Get(AI_MATKEY_COLOR_EMISSIVE, color) != aiReturn_SUCCESS) {
            m.data.emissive_color = glm::vec4(0.0f);
        } else {
            m.data.emissive_color = glm::vec4(color.r, color.g, color.b, 1.0f);
        }
        if (assimp_material->Get(AI_MATKEY_COLOR_TRANSPARENT, color) != aiReturn_SUCCESS) {
            m.data.transparent_color = glm::vec4(0.0f);
        } else {
            m.data.transparent_color = glm::vec4(color.r, color.g, color.b, 1.0f);
        }
        return m;
    }

    SceneData SceneLoader::load_scene_from(const std::filesystem::path& to_scene, unsigned int postprocessing_flags) {
        SceneLoader loader(to_scene,
                           postprocessing_flags);
        return loader.process_scene();
    }

    SceneLoader::SceneLoader(std::filesystem::path scene_path, unsigned int flags)
            : path_to_scene(std::move(scene_path)),
              assimp_postprocessing_flags(flags) {}

    SceneData SceneLoader::process_scene() {
        Assimp::Importer scene_importer;
        const aiScene* loaded_scene = scene_importer.ReadFile(path_to_scene.string(), assimp_postprocessing_flags);

        if (loaded_scene == nullptr) {
            fmt::print("[SCENE LOADING] Failed to read the scene from file");
            return SceneData();
        }

        scene_objects.reserve(loaded_scene->mNumMeshes);
        for (auto child_index = 0u; child_index < loaded_scene->mRootNode->mNumChildren; ++child_index) {
            process_scene_node(loaded_scene,
                               loaded_scene->mRootNode->mChildren[child_index],
                               loaded_scene->mRootNode->mTransformation);
        }
        return SceneData{std::move(scene_objects), std::move(scene_textures)};
    }

    //  We know this function is recursive and it is terminating; ignore Clang-Tidy's warning about it
    void SceneLoader::process_scene_node(const aiScene* scene,
                                         const aiNode* node,
                                         const aiMatrix4x4& parent_transform) {
        const auto node_transform = parent_transform * node->mTransformation;
        for (auto i = 0u; i < node->mNumMeshes; ++i) {
            const auto mesh_index = node->mMeshes[i];
            scene_objects.push_back(process_mesh(scene, scene->mMeshes[mesh_index], node_transform));
        }

        for (auto child_index = 0u; child_index < node->mNumChildren; ++child_index) {
            process_scene_node(scene,
                               node->mChildren[child_index],
                               node_transform);
        }
    }

    //TODO: rename as create_scene_object_from(mesh, mesh_transform, source_scene)
    SceneObject SceneLoader::process_mesh(const aiScene* source_scene,
                                          const aiMesh* mesh,
                                          const aiMatrix4x4& mesh_transform) {
        const unsigned int material_index = mesh->mMaterialIndex;
        const aiMaterial* assimp_material = source_scene->mMaterials[material_index];

        //TODO: check if it has uv coordinates and a diffuse texture, then load vertex data
//        aiString diffuse_texture_path{};
//        const auto result = assimp_material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_texture_path);
//        const auto success = (result == 0);
        std::vector<float> vertices = get_vertex_data(mesh);
        std::vector<unsigned int> indices = get_indices(mesh);

        auto vbo = std::make_shared<VertexBuffer>(vertices.size() * sizeof(float),
                                                  vertices.data());
        vbo->set_buffer_layout(VertexBufferLayout({
                                                          VertexBufferElement(ShaderDataType::Float3,
                                                                              "position"),
                                                          VertexBufferElement(ShaderDataType::Float3,
                                                                              "normal")}));

        SceneObject obj;
        obj.set_transform_matrix(aiMatrix4x4(mesh_transform));
        obj.vao->set_vbo(std::move(vbo));
        obj.vao->set_ebo(std::make_shared<ElementBuffer>(indices));
        obj.material = convert_assimp_material(assimp_material);

        return obj;
    }

    std::vector<float> get_vertex_data(const aiMesh* source) {
        std::vector<float> data;
        for (auto i = 0u; i < source->mNumVertices; ++i) {
            const auto& position = source->mVertices[i];
            const auto& normal = source->mNormals[i];
            //TODO: add uv coords here
            data.push_back(position.x);
            data.push_back(position.y);
            data.push_back(position.z);
            data.push_back(normal.x);
            data.push_back(normal.y);
            data.push_back(normal.z);
        }
        return data;
    }

    std::vector<unsigned int> get_indices(const aiMesh* source) {
        std::vector<unsigned int> indices;
        for (auto i = 0u; i < source->mNumFaces; ++i) {
            const auto& triangle = source->mFaces[i];
            for (auto j = 0u; j < triangle.mNumIndices; ++j) {
                indices.push_back(triangle.mIndices[j]);
            }
        }
        return indices;
    }
}