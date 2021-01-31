#include "scene_layer.h"

namespace engine{
    void ModelData::set_transform_matrix(aiMatrix4x4 model_matrix){
        const auto& m = model_matrix.Transpose();

        transform = glm::mat4(m.a1, m.a2, m.a3, m.a4,
                              m.b1, m.b2, m.b3, m.b4,
                              m.c1, m.c2, m.c3, m.c4,
                              m.d1, m.d2, m.d3, m.d4);
    }

    std::unique_ptr<ModelData> process_mesh(const aiMesh* mesh, const aiMatrix4x4& transform){
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        for(auto i = 0u; i < mesh->mNumVertices; ++i){
            const auto& position = mesh->mVertices[i];
            const auto& normal = mesh->mNormals[i];
            vertices.push_back(position.x);
            vertices.push_back(position.y);
            vertices.push_back(position.z);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }

        for(auto i = 0u; i < mesh->mNumFaces; ++i){
            const auto& triangle = mesh->mFaces[i];
            for(auto j = 0u; j < triangle.mNumIndices; ++j){
                indices.push_back(triangle.mIndices[j]);
            }
        }

        auto vbo = std::make_shared<VertexBuffer>(vertices.size() * sizeof(float),
                                                  vertices.data());
        vbo->set_buffer_layout(VertexBufferLayout({
                                                          VertexBufferElement(ShaderDataType::Float3,
                                                                              "position"),
                                                          VertexBufferElement(ShaderDataType::Float3,
                                                                              "normal")}));
        auto model = std::make_unique<ModelData>();
        model->set_transform_matrix(aiMatrix4x4(transform));
        model->vao.set_vbo(std::move(vbo));
        model->vao.set_ebo(std::make_shared<ElementBuffer>(indices));
        return model;
    }

    std::vector<std::unique_ptr<ModelData>> process_scene_node(const aiScene* scene, const aiNode* node,
                                                               const aiMatrix4x4& parent_transform){
        std::vector<std::unique_ptr<ModelData>> meshes;

        const auto node_transform = parent_transform * node->mTransformation;
        for (auto i = 0u; i < node->mNumMeshes; ++i) {
            const auto mesh_index = node->mMeshes[i];
            meshes.emplace_back(process_mesh(scene->mMeshes[mesh_index], node_transform));
        }

        for (auto child_index = 0u; child_index < node->mNumChildren; ++child_index) {
            auto processing_result = process_scene_node(scene,
                                                        node->mChildren[child_index],
                                                        node_transform);
            std::move(std::begin(processing_result), std::end(processing_result),
                      std::back_inserter(meshes));
        }
        return meshes;
    }

    void SceneLayer::load_from_scene(const aiScene* scene) {
        models.reserve(scene->mNumMeshes);
        for (auto child_index = 0u; child_index < scene->mRootNode->mNumChildren; ++child_index) {
            auto processing_result = process_scene_node(scene,
                                                        scene->mRootNode->mChildren[child_index],
                                                        scene->mRootNode->mTransformation);
            std::move(std::begin(processing_result), std::end(processing_result),
                      std::back_inserter(models));
        }
    }

    void SceneLayer::on_attach() {
        {
            Assimp::Importer test_importer;
            const aiScene* cornell_box_scene = test_importer.ReadFile("resources/cornell_box_multimaterial.obj",
                                                                      aiProcess_Triangulate | aiProcess_GenNormals);
            if(cornell_box_scene != nullptr && cornell_box_scene->HasMeshes()){
                load_from_scene(cornell_box_scene);
            }

            view_camera = Camera(CameraGeometricDefinition{.position{278.0f, 273.0f, -800.0f},
                                                           .look_at_position{278.0f, 273.0f, 0.0f},
                                                           .up{0.0f, 1.0f, 0.0f}}, 45.0f, 1.0f,
                                 CameraPlanes{0.1f, 10000.0f}, CameraMode::Perspective);

            const auto vertex_path = files::make_path_absolute("resources/shaders/base.vert");
            const auto fragment_path = files::make_path_absolute("resources/shaders/base.frag");
            auto vertex_source = files::read_file(vertex_path);
            auto fragment_source = files::read_file(fragment_path);
            const std::string vertex_filename(vertex_path.filename().string());
            const std::string fragment_filename(fragment_path.filename().string());
            shader = std::make_shared<Shader>(std::move(vertex_source), std::move(fragment_source),
                            std::string_view(vertex_filename), std::string_view(fragment_filename));

            color = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);

        }
    }

    void SceneLayer::on_detach() {}

    void engine::SceneLayer::on_event(engine::Event& event) {
        event.handled = false;
    }

    void SceneLayer::update(float delta_time) {
        Layer::update(delta_time);
        OpenGL3_Renderer::clear();
        shader->use();
        shader->set_mat4("view", view_camera.get_view_matrix());
        shader->set_mat4("projection", view_camera.get_projection_matrix());
        shader->set_vec4("color", color);
        if(!models.empty()){
            for(const auto& drawable : models){
                shader->set_mat4("model", drawable->transform);
                OpenGL3_Renderer::draw(drawable->vao);
            }
        }
    }

    void SceneLayer::on_imgui_render() {
        Layer::on_imgui_render();
    }
}


