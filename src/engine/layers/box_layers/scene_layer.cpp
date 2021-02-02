#include "scene_layer.h"

namespace engine{
    void SceneLayer::on_attach() {
        {
            constexpr unsigned int ai_postprocess_flags = aiProcess_GenNormals |
                                                          aiProcess_Triangulate |
                                                          aiProcess_ValidateDataStructure;
            scene_objects = scenes::load_scene_objects_from("resources/cornell_box_multimaterial.obj", ai_postprocess_flags);

            view_camera = Camera(CameraGeometricDefinition{.position{278.0f, 273.0f, -800.0f},
                                                           .look_at_position{278.0f, 273.0f, 0.0f},
                                                           .up{0.0f, 1.0f, 0.0f}}, 45.0f, 1.0f,
                                 CameraPlanes{0.1f, 10000.0f}, CameraMode::Perspective);

            light_position = glm::vec3(278.0f, 548.0f, 279.5f);

            const auto vertex_path = files::make_path_absolute("resources/shaders/base.vert");
            const auto fragment_path = files::make_path_absolute("resources/shaders/base.frag");
            auto vertex_source = files::read_file(vertex_path);
            auto fragment_source = files::read_file(fragment_path);
            const std::string vertex_filename(vertex_path.filename().string());
            const std::string fragment_filename(fragment_path.filename().string());
            shader = std::make_shared<Shader>(std::move(vertex_source), std::move(fragment_source),
                            std::string_view(vertex_filename), std::string_view(fragment_filename));
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
        shader->set_vec3("camera_position", view_camera.get_position());
        shader->set_vec3("light_position", light_position);
        if(!scene_objects.empty()){
            for(const auto& drawable : scene_objects){
                drawable.material.bind_uniforms_to(shader);
                shader->set_mat4("model", drawable.transform);
                OpenGL3_Renderer::draw(*(drawable.vao));
            }
        }
    }

    void SceneLayer::on_imgui_render() {
        Layer::on_imgui_render();
    }
}


