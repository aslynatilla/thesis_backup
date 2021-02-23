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
                                 CameraPlanes{0.1f, 2000.0f}, CameraMode::Perspective);

            scene_light = SpotLight(glm::vec3(278.0f, 548.0f, 279.5f),
                                    glm::vec3(0.0f, -1.0f, 0.0f),
                                    70.0f, 80.0f,
                                    1.0f, 0.004f, 0.00009f);

            draw_shader = shader::create_shader_from("resources/shaders/shadowmapped.vert",
                                                     "resources/shaders/shadowmapped.frag");
            rsm_generation_shader = shader::create_shader_from("resources/shaders/rsm.vert",
                                                      "resources/shaders/rsm.frag");

            auto viewport_float_dimension = std::make_unique<float[]>(4);
            glGetFloatv(GL_VIEWPORT, viewport_float_dimension.get());
            viewport_dimension[0] = static_cast<unsigned int>(viewport_float_dimension[0]);
            viewport_dimension[1] = static_cast<unsigned int>(viewport_float_dimension[1]);
            viewport_dimension[2] = static_cast<unsigned int>(viewport_float_dimension[2]);
            viewport_dimension[3] = static_cast<unsigned int>(viewport_float_dimension[3]);

            texture_dimension =
                    {static_cast<unsigned int>(viewport_dimension[2]/2),
                     static_cast<unsigned int>(viewport_dimension[3])/2};

            rsm_fbo = std::make_unique<OpenGL3_FrameBuffer>();
            depth_texture = std::make_unique<OpenGL3_Texture>(GL_TEXTURE_2D, GL_DEPTH_COMPONENT,
                                                              OpenGL3_TextureParameters(
                                                                      {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                       GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                      {GL_LINEAR, GL_LINEAR,
                                                                       GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}),
                                                              texture_dimension[0],
                                                              texture_dimension[1],
                                                              GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            position_texture = std::make_unique<OpenGL3_Texture>(GL_TEXTURE_2D, GL_RGB32F,
                                                                 OpenGL3_TextureParameters(
                                                                    {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                     GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                    {GL_LINEAR, GL_LINEAR,
                                                                     GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}),
                                                                 texture_dimension[0],
                                                                 texture_dimension[1],
                                                                 GL_RGB, GL_FLOAT, nullptr);
            normal_texture = std::make_unique<OpenGL3_Texture>(GL_TEXTURE_2D, GL_RGB32F,
                                                               OpenGL3_TextureParameters(
                                                                       {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                        GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                       {GL_LINEAR, GL_LINEAR,
                                                                        GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}),
                                                               texture_dimension[0],
                                                               texture_dimension[1],
                                                               GL_RGB, GL_FLOAT, nullptr);
            flux_texture = std::make_unique<OpenGL3_Texture>(GL_TEXTURE_2D, GL_RGB8,
                                                             OpenGL3_TextureParameters(
                                                                     {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                      GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                     {GL_LINEAR, GL_LINEAR,
                                                                      GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}),
                                                             texture_dimension[0],
                                                             texture_dimension[1],
                                                             GL_RGB, GL_FLOAT, nullptr);

            samples_number = 400;
            const auto samples = random_num::random_polar_offsets(samples_number);
            glGenTextures(1, &random_samples_texture);
            glBindTexture(GL_TEXTURE_1D, random_samples_texture);
            glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB32F, samples_number, 0, GL_RGB, GL_FLOAT, samples.data());
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

            glBindFramebuffer(GL_FRAMEBUFFER, rsm_fbo->id);
            rsm_fbo->bind_as(GL_FRAMEBUFFER);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *depth_texture);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *position_texture);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, *normal_texture);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, *flux_texture);
            const auto buffer_enums = std::make_unique<GLenum[]>(3);
            buffer_enums[0] = GL_COLOR_ATTACHMENT0;
            buffer_enums[1] = GL_COLOR_ATTACHMENT1;
            buffer_enums[2] = GL_COLOR_ATTACHMENT2;
            glDrawBuffers(3, buffer_enums.get());
            rsm_fbo->unbind_from();
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
//            glEnable(GL_CULL_FACE);
//            glCullFace(GL_BACK);
//            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }
    }

    void SceneLayer::on_detach() {
        glDeleteTextures(1, &random_samples_texture);
    }

    void engine::SceneLayer::on_event(engine::Event& event) {
//        Handle this later - resize texture when viewport resized?
//        EventHandler handler(event);
//        handler.handle<WindowResizedEvent>([this](auto&& ... args) -> decltype(auto) {
//            return on_window_resized(std::forward<decltype(args)>(args)...);
//        });
        event.handled = false;
    }

    void SceneLayer::update(float delta_time) {
        Layer::update(delta_time);

        rsm_fbo->bind_as(GL_FRAMEBUFFER);
        rsm_generation_shader->use();
        rsm_generation_shader->set_vec3("scene_light.position", scene_light.position);
        rsm_generation_shader->set_vec3("scene_light.direction", scene_light.direction);
        rsm_generation_shader->set_float("scene_light.cutoff_angle", scene_light.cosine_cutoff_angle);
        rsm_generation_shader->set_float("scene_light.outer_cutoff_angle", scene_light.cosine_outer_cutoff_angle);
        rsm_generation_shader->set_float("scene_light.constant_attenuation", scene_light.constant_attenuation_factor);
        rsm_generation_shader->set_float("scene_light.linear_attenuation", scene_light.linear_attenuation_factor);
        rsm_generation_shader->set_float("scene_light.quadratic_attenuation", scene_light.quadratic_attenuation_factor);
        rsm_generation_shader->set_float("light_intensity", light_intensity);

        glViewport(0, 0, texture_dimension[0], texture_dimension[1]);
        const auto light_camera = Camera(
                CameraGeometricDefinition{scene_light.position,
                                          scene_light.position + scene_light.direction,
                                          glm::vec3(0.0f, 0.0f, -1.0f)},
                90.0f, 1.0f,
                CameraPlanes{0.1f, 2000.0f},
                CameraMode::Perspective);

        const auto light_view_matrix = light_camera.get_view_matrix();
        const auto light_projection_matrix = light_camera.get_projection_matrix();
        rsm_generation_shader->set_mat4("light_view", light_view_matrix);
        rsm_generation_shader->set_mat4("light_projection", light_projection_matrix);
        rsm_generation_shader->set_float("far_plane", 2000.0f);
        rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                             GL_TEXTURE_2D,
                                             depth_texture->id);
        rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                             GL_TEXTURE_2D,
                                             position_texture->id);
        rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                                             GL_TEXTURE_2D,
                                             normal_texture->id);
        rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
                                             GL_TEXTURE_2D,
                                             flux_texture->id);
        OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_BLEND);
        if (!scene_objects.empty()) {
            for (const auto& drawable : scene_objects) {
                rsm_generation_shader->set_mat4("model", drawable.transform);
                rsm_generation_shader->set_vec4("diffuse_color", drawable.material.diffuse_color);
                OpenGL3_Renderer::draw(*(drawable.vao));
            }
        }

        rsm_fbo->unbind_from(GL_FRAMEBUFFER);
        OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        OpenGL3_Renderer::clear();
        glViewport(0, 0, viewport_dimension[2], viewport_dimension[3]);
        draw_shader->use();
        draw_shader->set_mat4("light_view", light_view_matrix);
        draw_shader->set_mat4("light_projection", light_projection_matrix);
        draw_shader->set_float("far_plane", 2000.0f);
        draw_shader->set_mat4("view", view_camera.get_view_matrix());
        draw_shader->set_mat4("projection", view_camera.get_projection_matrix());
        draw_shader->set_vec3("camera_position", view_camera.get_position());
        draw_shader->set_vec3("scene_light.position", scene_light.position);
        draw_shader->set_vec3("scene_light.direction", scene_light.direction);
        draw_shader->set_float("scene_light.cutoff_angle", scene_light.cosine_cutoff_angle);
        draw_shader->set_float("scene_light.outer_cutoff_angle", scene_light.cosine_outer_cutoff_angle);
        draw_shader->set_float("scene_light.constant_attenuation", scene_light.constant_attenuation_factor);
        draw_shader->set_float("scene_light.linear_attenuation", scene_light.linear_attenuation_factor);
        draw_shader->set_float("scene_light.quadratic_attenuation", scene_light.quadratic_attenuation_factor);

        draw_shader->set_int("shadow_map", 0);
        draw_shader->set_int("position_map", 1);
        draw_shader->set_int("normal_map", 2);
        draw_shader->set_int("flux_map", 3);
        draw_shader->set_int("sample_array", 4);

        //  TWEAKABLES
        draw_shader->set_float("light_intensity", light_intensity);
        draw_shader->set_float("indirect_intensity", indirect_intensity);
        draw_shader->set_float("max_radius", max_radius);

        depth_texture->make_active_in_slot(0);
        glBindTexture(GL_TEXTURE_2D, depth_texture->id);
        position_texture->make_active_in_slot(1);
        glBindTexture(GL_TEXTURE_2D, position_texture->id);
        normal_texture->make_active_in_slot(2);
        glBindTexture(GL_TEXTURE_2D, normal_texture->id);
        flux_texture->make_active_in_slot(3);
        glBindTexture(GL_TEXTURE_2D, flux_texture->id);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_1D, random_samples_texture);
        draw_shader->set_int("samples_number", samples_number);
        //glEnable(GL_BLEND);
        if (!scene_objects.empty()) {
            for (const auto& drawable : scene_objects) {
                drawable.material.bind_uniforms_to(draw_shader);
                draw_shader->set_mat4("model", drawable.transform);
                OpenGL3_Renderer::draw(*(drawable.vao));
            }
        }
    }

    void SceneLayer::on_imgui_render() {
        ImGui::Begin("Shader controls");
        ImGui::SliderFloat("Spotlight Intensity", &light_intensity, 0.5f, 5.0f);
        ImGui::SliderFloat("Indirect Component Intensity", &indirect_intensity, 1.0f, 10000.0f);
        ImGui::SliderFloat("Max radius sample", &max_radius, 10.0f, static_cast<float>(texture_dimension[0]));
        ImGui::End();
    }
}


