#include "scene_loading.h"

namespace engine::scenes{

    void copy_vertices(const aiMesh* source, std::vector<float>& destination){
        for(auto i = 0u; i < source->mNumVertices; ++i){
            const auto& position = source->mVertices[i];
            const auto& normal = source->mNormals[i];
            destination.push_back(position.x);
            destination.push_back(position.y);
            destination.push_back(position.z);
            destination.push_back(normal.x);
            destination.push_back(normal.y);
            destination.push_back(normal.z);
        }
    }

    void copy_indices(const aiMesh* source, std::vector<unsigned int>& destination){
        for(auto i = 0u; i < source->mNumFaces; ++i){
            const auto& triangle = source->mFaces[i];
            for(auto j = 0u; j < triangle.mNumIndices; ++j){
                destination.push_back(triangle.mIndices[j]);
            }
        }
    }

    Material convert_assimp_material(aiMaterial* assimp_material){
        Material m;
        aiString material_name;
        if(assimp_material->Get(AI_MATKEY_NAME, material_name) != aiReturn_SUCCESS){
            m.name = Material::default_name;
        } else {
            m.name = std::string(material_name.C_Str());
        }
        if(assimp_material->Get(AI_MATKEY_OPACITY, m.data.opacity) != aiReturn_SUCCESS){
            m.data.opacity = 1.0f;
        }
        if(assimp_material->Get(AI_MATKEY_SHININESS, m.data.shininess) != aiReturn_SUCCESS){
            m.data.shininess = 0.0f;
        }
        if(assimp_material->Get(AI_MATKEY_REFRACTI, m.data.refraction_index) != aiReturn_SUCCESS){
            m.data.refraction_index = 1.0f;
        }
        aiColor3D color;
        if(assimp_material->Get(AI_MATKEY_COLOR_DIFFUSE, color) != aiReturn_SUCCESS){
            m.data.diffuse_color = glm::vec4(0.0f);
        }
        else {
            m.data.diffuse_color = glm::vec4(color.r, color.g, color.b, 1.0f);
        }
        if(assimp_material->Get(AI_MATKEY_COLOR_AMBIENT, color) != aiReturn_SUCCESS){
            m.data.ambient_color = glm::vec4(0.0f);
        }
        else {
            m.data.ambient_color = glm::vec4(color.r, color.g, color.b, 1.0f);
        }
        if(assimp_material->Get(AI_MATKEY_COLOR_SPECULAR, color) != aiReturn_SUCCESS){
            m.data.specular_color = glm::vec4(0.0f);
        }
        else {
            m.data.specular_color = glm::vec4(color.r, color.g, color.b, 1.0f);
        }
        if(assimp_material->Get(AI_MATKEY_COLOR_EMISSIVE, color) != aiReturn_SUCCESS){
            m.data.emissive_color = glm::vec4(0.0f);
        }
        else {
            m.data.emissive_color = glm::vec4(color.r, color.g, color.b, 1.0f);
        }
        if(assimp_material->Get(AI_MATKEY_COLOR_TRANSPARENT, color) != aiReturn_SUCCESS){
            m.data.transparent_color = glm::vec4(0.0f);
        }
        else {
            m.data.transparent_color = glm::vec4(color.r, color.g, color.b, 1.0f);
        }
        return m;
    }

    SceneObject process_mesh(const aiScene* source_scene, const aiMesh* mesh, const aiMatrix4x4& mesh_transform){
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        copy_vertices(mesh, vertices);
        copy_indices(mesh, indices);


        auto vbo = std::make_shared<VertexBuffer>(vertices.size() * sizeof(float),
                                                  vertices.data());
        vbo->set_buffer_layout(VertexBufferLayout({
                                                          VertexBufferElement(ShaderDataType::Float3,
                                                                              "position"),
                                                          VertexBufferElement(ShaderDataType::Float3,
                                                                              "normal")}));

        const unsigned int material_index = mesh->mMaterialIndex;
        aiMaterial* assimp_material = source_scene->mMaterials[material_index];

        SceneObject obj;
        obj.set_transform_matrix(aiMatrix4x4(mesh_transform));
        obj.vao->set_vbo(std::move(vbo));
        obj.vao->set_ebo(std::make_shared<ElementBuffer>(indices));
        obj.material = convert_assimp_material(assimp_material);
        aiString diffuse_texture_path{};
        const auto result = assimp_material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_texture_path);
        const auto success = (result == 0);

        return obj;
    }

    std::vector<SceneObject> process_scene_node(const aiScene* scene, const aiNode* node,
                                                               const aiMatrix4x4& parent_transform){
        std::vector<SceneObject> objects;

        const auto node_transform = parent_transform * node->mTransformation;
        for (auto i = 0u; i < node->mNumMeshes; ++i) {
            const auto mesh_index = node->mMeshes[i];
            objects.emplace_back(process_mesh(scene, scene->mMeshes[mesh_index], node_transform));
        }

        for (auto child_index = 0u; child_index < node->mNumChildren; ++child_index) {
            auto processing_result = process_scene_node(scene,
                                                        node->mChildren[child_index],
                                                        node_transform);
            std::move(std::begin(processing_result), std::end(processing_result),
                      std::back_inserter(objects));
        }
        return objects;
    }

    std::vector<SceneObject> load_scene_objects_from(const std::string& path_to_scene, unsigned int assimp_postprocess_flags) {
        Assimp::Importer scene_importer;
        const aiScene* loaded_scene = scene_importer.ReadFile(path_to_scene, assimp_postprocess_flags);

        if(loaded_scene == nullptr){
            fmt::print("[SCENE LOADING] Reading scene failed.\n");
            return std::vector<SceneObject>{};
        }

        std::vector<SceneObject> objects;
        objects.reserve(loaded_scene->mNumMeshes);

        for (auto child_index = 0u; child_index < loaded_scene->mRootNode->mNumChildren; ++child_index) {
            auto processing_result = process_scene_node(loaded_scene,
                                                        loaded_scene->mRootNode->mChildren[child_index],
                                                        loaded_scene->mRootNode->mTransformation);
            std::move(std::begin(processing_result), std::end(processing_result),
                      std::back_inserter(objects));
        }
        return objects;
    }


}