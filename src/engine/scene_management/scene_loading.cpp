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
              assimp_postprocessing_flags(flags) {
        scene_directory = path_to_scene.parent_path();
    }

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
            scene_objects.push_back(create_scene_object_from(scene->mMeshes[mesh_index], node_transform, scene));
        }

        for (auto child_index = 0u; child_index < node->mNumChildren; ++child_index) {
            process_scene_node(scene,
                               node->mChildren[child_index],
                               node_transform);
        }
    }

    SceneObject SceneLoader::create_scene_object_from(const aiMesh* mesh, const aiMatrix4x4& mesh_transform,
                                                      const aiScene* source_scene) {
        const unsigned int material_index = mesh->mMaterialIndex;
        const aiMaterial* assimp_material = source_scene->mMaterials[material_index];
        SceneObject obj;

        if (aiString diffuse_texture_path{}; mesh->HasTextureCoords(0) &&
                                             assimp_material->GetTexture(aiTextureType_DIFFUSE, 0,
                                                                         &diffuse_texture_path) == aiReturn_SUCCESS) {
            //  NOTE: operator/ is necessary since it returns a new path, while append would modify the first parameter
            std::filesystem::path abs_path_to_texture = scene_directory / diffuse_texture_path.data;
            abs_path_to_texture = files::make_path_absolute(abs_path_to_texture.string());
            std::string tex_path = abs_path_to_texture.string();
            std::size_t hashed_path = std::hash<std::string>{}(tex_path);
            if(auto it = std::find(texture_path_hashes.begin(), texture_path_hashes.end(), hashed_path);
                it == texture_path_hashes.end()){
                texture_path_hashes.push_back(hashed_path);
                auto load_result = load_texture(abs_path_to_texture.string());
                auto texture = OpenGL3_Texture2D_Builder()
                        .with_size(load_result.width, load_result.height)
                        .with_texture_format(GL_RGBA8)
                        .with_data_format(GL_RGBA)
                        .using_underlying_data_type(GL_UNSIGNED_BYTE)
                        .using_linear_magnification()
                        .using_linear_minification()
                        .as_resource_with_data(load_result.data.data());
                scene_textures.emplace_back(std::move(texture));
                obj.texture_index = static_cast<int>(texture_path_hashes.size() - 1u);
            } else {
                obj.texture_index = std::distance(texture_path_hashes.begin(), it);
            }

        }

        std::vector<float> vertices = get_vertex_data(mesh);
        std::vector<unsigned int> indices = get_indices(mesh);

        auto vbo = std::make_shared<VertexBuffer>(vertices.size() * sizeof(float),
                                                  vertices.data());

        VertexBufferLayout layout = compute_vertex_buffer_layout(mesh);
        vbo->set_buffer_layout(layout);

        obj.set_transform_matrix(aiMatrix4x4(mesh_transform));
        obj.vao->set_vbo(std::move(vbo));
        obj.vao->set_ebo(std::make_shared<ElementBuffer>(indices));
        obj.material = convert_assimp_material(assimp_material);
        return obj;
    }

    VertexBufferLayout compute_vertex_buffer_layout(const aiMesh* target_mesh) {
        VertexBufferLayout layout;
        if (target_mesh->HasTextureCoords(0)) {
            layout = VertexBufferLayout(
                    {VertexBufferElement(ShaderDataType::Float3, "position"),
                     VertexBufferElement(ShaderDataType::Float3, "normal"),
                     VertexBufferElement(ShaderDataType::Float2, "uv")
                    });
        } else {
            layout = VertexBufferLayout(
                    {VertexBufferElement(ShaderDataType::Float3, "position"),
                     VertexBufferElement(ShaderDataType::Float3, "normal")}
            );
        }
        return layout;
    }

    std::vector<float> extract_vertex_data(int vertices_number,
                                           aiVector3D* positions,
                                           aiVector3D* normals) {
        std::vector<float> data;
        for (auto i = 0; i < vertices_number; ++i) {
            std::copy_n(&(positions[i].x), 3, std::back_inserter(data));
            std::copy_n(&(normals[i].x), 3, std::back_inserter(data));
        }
        return data;
    }

    std::vector<float> extract_vertex_data(int vertices_number,
                                           aiVector3D* positions,
                                           aiVector3D* normals,
                                           aiVector3D* uv_coords) {
        std::vector<float> data;
        for (auto i = 0; i < vertices_number; ++i) {
            std::copy_n(&(positions[i].x), 3, std::back_inserter(data));
            std::copy_n(&(normals[i].x), 3, std::back_inserter(data));
            std::copy_n(&(uv_coords[i].x), 2, std::back_inserter(data));
        }
        return data;
    }

    std::vector<float> get_vertex_data(const aiMesh* source) {
        if (source->HasTextureCoords(0)) {
            return extract_vertex_data(source->mNumVertices,
                                       source->mVertices, source->mNormals, source->mTextureCoords[0]);
        } else {
            return extract_vertex_data(source->mNumVertices,
                                       source->mVertices, source->mNormals);
        }
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

    TextureLoadResult load_texture(std::string_view texture_path) {
        TextureLoadResult result;
        auto tex = stbi_load(texture_path.data(), &result.width, &result.height, &result.components_per_pixel, 4);
        if (tex != nullptr) {
            const auto pixel_num = result.width * result.height;
            std::copy_n(tex, pixel_num * 4, std::back_inserter(result.data));
            stbi_image_free(tex);
        }
        return result;
    }
}