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

            point_light.position = glm::vec3(278.0f, 548.0f, 279.5f);

            draw_shader = shader::create_shader_from("resources/shaders/shadowmapped.vert",
                                                     "resources/shaders/shadowmapped.frag");
            rsm_generation_shader = shader::create_shader_from("resources/shaders/rsm.vert",
                                                               "resources/shaders/rsm.frag");

            auto viewport_dimensions = std::make_unique<float[]>(4);
            glGetFloatv(GL_VIEWPORT, viewport_dimensions.get());

            rsm_fbo = std::make_unique<OpenGL3_FrameBuffer>();
            depth_cubemap = std::make_unique<OpenGL3_Cubemap>(GL_DEPTH_COMPONENT,
                                                              OpenGL3_TextureParameters(
                                                                      {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                       GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                      {GL_LINEAR, GL_LINEAR,
                                                                       GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}),
                                                              static_cast<unsigned int>(viewport_dimensions[2]),
                                                              static_cast<unsigned int>(viewport_dimensions[3]),
                                                              GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            pos_cubemap = std::make_unique<OpenGL3_Cubemap>(GL_RGB32F,
                                                            OpenGL3_TextureParameters(
                                                                    {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                     GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T,
                                                                     GL_TEXTURE_WRAP_R},
                                                                    {GL_LINEAR, GL_LINEAR,
                                                                     GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
                                                                     GL_CLAMP_TO_EDGE}),
                                                            static_cast<unsigned int>(viewport_dimensions[2]),
                                                            static_cast<unsigned int>(viewport_dimensions[3]),
                                                            GL_RGB, GL_FLOAT, nullptr);
            normal_cubemap = std::make_unique<OpenGL3_Cubemap>(GL_RGB32F,
                                                               OpenGL3_TextureParameters(
                                                                       {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                        GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T,
                                                                        GL_TEXTURE_WRAP_R},
                                                                       {GL_LINEAR, GL_LINEAR,
                                                                        GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
                                                                        GL_CLAMP_TO_EDGE}),
                                                               static_cast<unsigned int>(viewport_dimensions[2]),
                                                               static_cast<unsigned int>(viewport_dimensions[3]),
                                                               GL_RGB, GL_FLOAT, nullptr);
            flux_cubemap = std::make_unique<OpenGL3_Cubemap>(GL_RGB32F,
                                                             OpenGL3_TextureParameters(
                                                                     {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                      GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T,
                                                                      GL_TEXTURE_WRAP_R},
                                                                     {GL_LINEAR, GL_LINEAR,
                                                                      GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
                                                                      GL_CLAMP_TO_EDGE}),
                                                             static_cast<unsigned int>(viewport_dimensions[2]),
                                                             static_cast<unsigned int>(viewport_dimensions[3]),
                                                             GL_RGB, GL_FLOAT, nullptr);

            //  1 => x, 2 => y, 3 => z
            //  prefix minus (extract with sign) if needs inverting
            //  subtract 1 to abs() to get the component
            std::array<glm::vec<2, glm::i32, glm::lowp>, 6> offset_components_per_face{
                    glm::vec<2, glm::i32, glm::lowp>{-3, -2},
                    glm::vec<2, glm::i32, glm::lowp>{3, -2},
                    glm::vec<2, glm::i32, glm::lowp>{1, 3},
                    glm::vec<2, glm::i32, glm::lowp>{1, -3},
                    glm::vec<2, glm::i32, glm::lowp>{1, -2},
                    glm::vec<2, glm::i32, glm::lowp>{-1, -2}

            };

            unsigned int offset_mask_cubemap_id = 0;
            glGenTextures(1, &offset_mask_cubemap_id);
            glBindTexture(GL_TEXTURE_CUBE_MAP, offset_mask_cubemap_id);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RG32I, 1, 1, 0, GL_RG_INTEGER, GL_INT, &(offset_components_per_face[0]));
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RG32I, 1, 1, 0, GL_RG_INTEGER, GL_INT, &(offset_components_per_face[1]));
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RG32I, 1, 1, 0, GL_RG_INTEGER, GL_INT, &(offset_components_per_face[2]));
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RG32I, 1, 1, 0, GL_RG_INTEGER, GL_INT, &(offset_components_per_face[3]));
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RG32I, 1, 1, 0, GL_RG_INTEGER, GL_INT, &(offset_components_per_face[4]));
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RG32I, 1, 1, 0, GL_RG_INTEGER, GL_INT, &(offset_components_per_face[5]));
            [[maybe_unused]] const auto error = glGetError() != 0;
            offset_mask_cubemap = std::make_unique<OpenGL3_Cubemap>(offset_mask_cubemap_id, 1, 1);

            sample_number = 50;
            auto polar_offsets = random_num::random_polar_offsets(sample_number);
            sample_texture = std::make_unique<OpenGL3_Texture1D>(GL_RGB32F, OpenGL3_TextureParameters(
                    {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER, GL_TEXTURE_WRAP_S},
                    {GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE}),
                                                                 sample_number,
                                                                 GL_RGB, GL_FLOAT, polar_offsets.data());


            glBindFramebuffer(GL_FRAMEBUFFER, rsm_fbo->id);
            rsm_fbo->bind_as(GL_FRAMEBUFFER);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *depth_cubemap);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *pos_cubemap);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, *normal_cubemap);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, *flux_cubemap);
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
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }
    }

    void SceneLayer::on_detach() {}

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

        const auto& pos = point_light.position;

        rsm_fbo->bind_as(GL_FRAMEBUFFER);
        rsm_generation_shader->use();
        rsm_generation_shader->set_vec3("light_position", pos);
        for (auto i = 0; i < 6; ++i) {
            const auto point_light_camera = Camera(
                    CameraGeometricDefinition{pos, pos + OpenGL3_Cubemap::directions[i], OpenGL3_Cubemap::ups[i]},
                    90.0f, 1.0f,
                    CameraPlanes{0.1f, 2000.0f},
                    CameraMode::Perspective);
            rsm_generation_shader->set_mat4("point_light_view", point_light_camera.get_view_matrix());
            rsm_generation_shader->set_mat4("point_light_projection", point_light_camera.get_projection_matrix());
            rsm_generation_shader->set_float("far_plane", 2000.0f);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                                 GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                                 depth_cubemap->id);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                 GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                                 pos_cubemap->id);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                                                 GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                                 normal_cubemap->id);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
                                                 GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                                 flux_cubemap->id);
            OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_BLEND);
            if (!scene_objects.empty()) {
                for (const auto& drawable : scene_objects) {
                    rsm_generation_shader->set_mat4("model", drawable.transform);
                    rsm_generation_shader->set_vec4("diffuse_color", drawable.material.diffuse_color);
                    OpenGL3_Renderer::draw(*(drawable.vao));
                }
            }
        }
        rsm_fbo->unbind_from(GL_FRAMEBUFFER);
        OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        OpenGL3_Renderer::clear();
        draw_shader->use();
        draw_shader->set_float("far_plane", 2000.0f);
        draw_shader->set_mat4("view", view_camera.get_view_matrix());
        draw_shader->set_mat4("projection", view_camera.get_projection_matrix());
        draw_shader->set_vec3("camera_position", view_camera.get_position());
        draw_shader->set_vec3("light_position", point_light.position);
        depth_cubemap->make_active_in_slot(0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap->id);
        pos_cubemap->make_active_in_slot(1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, pos_cubemap->id);
        normal_cubemap->make_active_in_slot(2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, normal_cubemap->id);
        flux_cubemap->make_active_in_slot(3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, flux_cubemap->id);
        sample_texture->make_active_in_slot(4);
        glBindTexture(GL_TEXTURE_1D, sample_texture->id);
        offset_mask_cubemap->make_active_in_slot(5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, offset_mask_cubemap->id);

        draw_shader->set_int("samples", sample_number);
        draw_shader->set_int("max_sample_radius", 20);

        if (!scene_objects.empty()) {
            for (const auto& drawable : scene_objects) {
                drawable.material.bind_uniforms_to(draw_shader);
                draw_shader->set_mat4("model", drawable.transform);
                OpenGL3_Renderer::draw(*(drawable.vao));
            }
        }
    }

    void SceneLayer::on_imgui_render() {
        Layer::on_imgui_render();
    }
}


