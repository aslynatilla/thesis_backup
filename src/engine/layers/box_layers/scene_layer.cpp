#include "scene_layer.h"

namespace engine {
    void SceneLayer::on_attach() {
        {
            constexpr unsigned int ai_postprocess_flags = aiProcess_GenNormals |
                                                          aiProcess_Triangulate |
                                                          aiProcess_ValidateDataStructure;
            scene_objects = scenes::load_scene_objects_from("resources/cornell_box_multimaterial.obj",
                                                            ai_postprocess_flags);

            view_camera = Camera(CameraGeometricDefinition{.position{278.0f, 270.0f, -800.0f},
                                         .look_at_position{278.0f, 273.0f, 0.0f},
                                         .up{0.0f, 1.0f, 0.0f}}, 45.0f, 1.0f,
                                 CameraPlanes{0.1f, 10000.0f}, CameraMode::Perspective);

            point_light.position = glm::vec3(278.0f, 547.0f, 279.5f);

            base_shader = shader::create_shader_from("resources/shaders/shadowmapped.vert",
                                                     "resources/shaders/shadowmapped.frag");
            depth_shader = shader::create_shader_from("resources/shaders/depth.vert",
                                                      "resources/shaders/depth.frag");

            auto viewport_dimensions = std::make_unique<float[]>(4);
            glGetFloatv(GL_VIEWPORT, viewport_dimensions.get());

            depth_framebuffer = std::make_unique<OpenGL3_FrameBuffer>();
            depth_texture = std::make_unique<OpenGL3_Texture>(GL_TEXTURE_2D, GL_DEPTH_COMPONENT32,
                                                              OpenGL3_TextureParameters(
                                                                      {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                       GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                      {GL_LINEAR, GL_LINEAR,
                                                                       GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}),
                                                              static_cast<unsigned int>(viewport_dimensions[2]), static_cast<unsigned int>(viewport_dimensions[3]),
                                                              GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

            glBindTexture(GL_TEXTURE_2D, 0);
            shadow_cubemap = std::make_unique<OpenGL3_Cubemap>(GL_R32F,
                                                               OpenGL3_TextureParameters(
                                                                  {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                   GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R},
                                                                  {GL_LINEAR, GL_LINEAR,
                                                                   GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}),
                                                               800, 800, GL_RED, GL_FLOAT, nullptr);

//            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, depth_framebuffer->id);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture->id, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, shadow_cubemap->id, 0);

//            depth_framebuffer->attach_texture_image_to(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
//                                                       depth_texture->bound_type, depth_texture->id);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
//            depth_framebuffer->attach_texture_image_to(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//                                                       shadow_cubemap->bound_type, shadow_cubemap->id);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
    }

    void SceneLayer::on_detach() {}

    void engine::SceneLayer::on_event(engine::Event& event) {
        event.handled = false;
    }

    void SceneLayer::update(float delta_time) {
        Layer::update(delta_time);
        //glCullFace(GL_FRONT);

        auto viewport_dimensions = std::make_unique<float[]>(4);
        glGetFloatv(GL_VIEWPORT, viewport_dimensions.get());
        glViewport(0, 0, shadow_cubemap->width, shadow_cubemap->height);

        depth_shader->use();
        const auto pos = point_light.position;
        depth_shader->set_vec3("light_position", pos);

        glClearColor(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, depth_framebuffer->id);
        for (auto i = 0; i < 6; ++i) {
            const auto point_light_camera = Camera(
                    CameraGeometricDefinition{pos, pos + OpenGL3_Cubemap::directions[i], OpenGL3_Cubemap::ups[i]},
                    90.0f, 1.0f,
                    CameraPlanes{0.1f, 1000.0f},
                    CameraMode::Perspective);
            depth_shader->set_mat4("light_view", point_light_camera.get_view_matrix());
            depth_shader->set_mat4("light_projection", point_light_camera.get_projection_matrix());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, depth_framebuffer->id);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, shadow_cubemap->id, 0);
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            if (!scene_objects.empty()) {
                for (const auto& drawable : scene_objects) {
                    depth_shader->set_mat4("model", drawable.transform);
                    OpenGL3_Renderer::draw(*(drawable.vao));
                }
            }
        }
        depth_framebuffer->unbind(GL_DRAW_FRAMEBUFFER);
        glCullFace(GL_BACK);
        glViewport(0, 0, static_cast<unsigned int>(viewport_dimensions[2]), static_cast<unsigned int>(viewport_dimensions[3]));
        OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        OpenGL3_Renderer::clear();
        base_shader->use();
        base_shader->set_mat4("view", view_camera.get_view_matrix());
        base_shader->set_mat4("projection", view_camera.get_projection_matrix());
        base_shader->set_vec3("camera_position", view_camera.get_position());
        base_shader->set_vec3("light_position", point_light.position);
        shadow_cubemap->make_active_in_slot(0);
        //base_shader->set_int("shadow_map", shadow_cubemap->id);
        if (!scene_objects.empty()) {
            for (const auto& drawable : scene_objects) {
                drawable.material.bind_uniforms_to(base_shader);
                base_shader->set_mat4("model", drawable.transform);
                OpenGL3_Renderer::draw(*(drawable.vao));
            }
        }
    }

    void SceneLayer::on_imgui_render() {
        Layer::on_imgui_render();
    }
}


