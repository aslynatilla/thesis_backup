#include "scene_layer.h"

namespace engine {

    SceneLayer::SceneLayer(std::weak_ptr<FlyCamera> application_camera)
            : view_camera(std::move(application_camera)) {}

    void SceneLayer::on_attach() {
        {
            constexpr unsigned int ai_postprocess_flags = aiProcess_GenNormals |
                                                          aiProcess_Triangulate |
                                                          aiProcess_ValidateDataStructure;
            scene_objects = scenes::load_scene_objects_from("resources/cornell_box_multimaterial.obj",
                                                            ai_postprocess_flags);
            //548.0f / 3.0f => ~185.0f
            const auto scene_scaling = glm::scale(glm::identity<glm::mat4>(), glm::vec3(1.0f/185.0f));
            const auto transposed_inverse_scene_scaling = glm::transpose(scene_scaling);
            for(auto&& object : scene_objects){
                const auto T = object.transform;
                object.transform = scene_scaling;
                object.transpose_inverse_transform = transposed_inverse_scene_scaling;
            }

            scene_light = Point_Light(glm::vec4(1.5f, 2.6f, 1.5f, 1.0f),
                                      LightAttenuationParameters{1.0f, 0.5f, 1.8f});
            scene_light.set_rotation(glm::vec3(90.0f, 0.0f, 0.0f));

            draw_shader = shader::create_shader_from("resources/shaders/shadowmapped.vert",
                                                     "resources/shaders/shadowmapped.frag");
            no_indirect_shader = shader::create_shader_from("resources/shaders/shadowmapped_no_indirect.vert",
                                                            "resources/shaders/shadowmapped_no_indirect.frag");
            rsm_generation_shader = shader::create_shader_from("resources/shaders/rsm.vert",
                                                               "resources/shaders/rsm.frag",
                                                               "resources/shaders/rsm.geom");
            wireframe_shader = shader::create_shader_from("resources/shaders/wireframe.vert",
                                                          "resources/shaders/wireframe.frag");
            depthmask_shader = shader::create_shader_from("resources/shaders/depth_mask.vert",
                                                          "resources/shaders/depth_mask.frag",
                                                          "resources/shaders/depth_mask.geom");

            auto viewport_float_dimension = std::make_unique<float[]>(4);
            glGetFloatv(GL_VIEWPORT, viewport_float_dimension.get());
            viewport_dimension[0] = static_cast<int>(viewport_float_dimension[0]);
            viewport_dimension[1] = static_cast<int>(viewport_float_dimension[1]);
            viewport_dimension[2] = static_cast<int>(viewport_float_dimension[2]);
            viewport_dimension[3] = static_cast<int>(viewport_float_dimension[3]);

            texture_dimension =
                    {static_cast<int>(viewport_dimension[2] / 4),
                     static_cast<int>(viewport_dimension[3]) / 4};

            light_transforms_strings = {
                    "light_transforms[0]",
                    "light_transforms[1]",
                    "light_transforms[2]",
                    "light_transforms[3]",
                    "light_transforms[4]",
                    "light_transforms[5]",
            };

            rsm_fbo = std::make_unique<OpenGL3_FrameBuffer>();
            mask_fbo = std::make_unique<OpenGL3_FrameBuffer>();
            depth_texture = std::make_unique<OpenGL3_Cubemap>(GL_DEPTH_COMPONENT,
                                                              OpenGL3_TextureParameters(
                                                                      {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                       GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                      {GL_LINEAR, GL_LINEAR,
                                                                       GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                              texture_dimension[0],
                                                              texture_dimension[1],
                                                              GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

            position_texture = std::make_unique<OpenGL3_Cubemap>(GL_RGB32F,
                                                                 OpenGL3_TextureParameters(
                                                                         {GL_TEXTURE_MIN_FILTER,
                                                                          GL_TEXTURE_MAG_FILTER,
                                                                          GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                         {GL_LINEAR, GL_LINEAR,
                                                                          GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                                 texture_dimension[0],
                                                                 texture_dimension[1],
                                                                 GL_RGB, GL_FLOAT, nullptr);

            normal_texture = std::make_unique<OpenGL3_Cubemap>(GL_RGB32F,
                                                               OpenGL3_TextureParameters(
                                                                       {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                        GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                       {GL_LINEAR, GL_LINEAR,
                                                                        GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                               texture_dimension[0],
                                                               texture_dimension[1],
                                                               GL_RGB, GL_FLOAT, nullptr);
            flux_texture = std::make_unique<OpenGL3_Cubemap>(GL_RGB8,
                                                             OpenGL3_TextureParameters(
                                                                     {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                      GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                     {GL_LINEAR, GL_LINEAR,
                                                                      GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                             texture_dimension[0],
                                                             texture_dimension[1],
                                                             GL_RGB, GL_FLOAT, nullptr);

            ies_light_mask = std::make_unique<OpenGL3_Cubemap>(GL_RGB32F,
                                                               OpenGL3_TextureParameters(
                                                                       {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                        GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                       {GL_LINEAR, GL_LINEAR,
                                                                        GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                               texture_dimension[0],
                                                               texture_dimension[1],
                                                               GL_RGB, GL_FLOAT, nullptr);


            samples_number = 100;
            const auto samples = random_num::random_directions(samples_number);

            //  TODO: consider using BufferTexture for this
            //  see: https://www.khronos.org/opengl/wiki/Buffer_Texture
            samples_texture = std::make_unique<OpenGL3_Texture1D>(GL_RGB32F,
                                                                  OpenGL3_TextureParameters(
                                                                          {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                           GL_TEXTURE_WRAP_S},
                                                                          {GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE}),
                                                                  samples_number,
                                                                  GL_RGB, GL_FLOAT, samples.data());

            //  Reflective Shadow Map Framebuffer setup
            rsm_fbo->bind_as(GL_FRAMEBUFFER);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *depth_texture);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *position_texture);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, *normal_texture);
            rsm_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, *flux_texture);
            //  Note: you need to specify which attachments the shader will be able to write to
            const auto buffer_enums = std::make_unique<GLenum[]>(3);
            buffer_enums[0] = GL_COLOR_ATTACHMENT0;
            buffer_enums[1] = GL_COLOR_ATTACHMENT1;
            buffer_enums[2] = GL_COLOR_ATTACHMENT2;
            glDrawBuffers(3, buffer_enums.get());
            rsm_fbo->unbind_from();
            mask_fbo->bind_as(GL_FRAMEBUFFER);
            mask_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *depth_texture);
            mask_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *ies_light_mask);
            glDrawBuffers(1, buffer_enums.get());
            mask_fbo->unbind_from(GL_FRAMEBUFFER);

            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

            //TODO: refactor as IES_Loader class or as a free function
            const auto path_to_IES_data = files::make_path_absolute("resources/ies/111621PN.IES");
            document = parser.parse(path_to_IES_data.filename().string(), files::read_file(path_to_IES_data));
            ies::adapter::IES_Mesh photometric_solid(document);

            const auto vertices = photometric_solid.get_vertices();

            auto farthest_vertex_distance = [](const std::vector<float>& vs) -> float {
                float result = 0.0f;
                for (auto i = 0u; i < vs.size() / 3; ++i) {
                    const glm::vec3 p(vs[3 * i + 0],
                                      vs[3 * i + 1],
                                      vs[3 * i + 2]);
                    const auto p_to_origin = glm::length(p);
                    if (result < p_to_origin) {
                        result = p_to_origin;
                    }
                }
                return result;
            };

            largest_position_component = farthest_vertex_distance(vertices);

            auto vbo = std::make_shared<VertexBuffer>(vertices.size() * sizeof(float),
                                                      vertices.data());
            vbo->set_buffer_layout(VertexBufferLayout({
                                                              VertexBufferElement(ShaderDataType::Float3,
                                                                                  "position"),
                                                              VertexBufferElement(ShaderDataType::Float3,
                                                                                  "normal")}));
            ies_light_vao.set_vbo(std::move(vbo));
            ies_light_vao.set_ebo(std::make_shared<ElementBuffer>(photometric_solid.get_indices()));
        }
    }

    void SceneLayer::on_detach() {}

    void engine::SceneLayer::on_event(engine::Event& event) {
        EventHandler handler(event);

//        Handle this later - resize texture when viewport resized?
//        TODO: implement a way to resize the texture
//        handler.handle<WindowResizedEvent>([this](auto&& ... args) -> decltype(auto) {
//            return on_window_resized(std::forward<decltype(args)>(args)...);
//        });

        handler.handle<KeyPressedEvent>([this](auto&& ...args) -> decltype(auto) {
            return on_key_pressed(std::forward<decltype(args)>(args)...);
        });
        event.handled = false;
    }

    void SceneLayer::update(float delta_time) {
        timestep = delta_time;
        if (auto existing_camera = view_camera.lock()) {
            const auto light_position = scene_light.get_position_as_vec3();
            const auto light_forward = glm::vec3(scene_light.get_forward());
            const auto light_orientation = glm::mat4_cast(scene_light.get_orientation());
            const auto light_camera = Camera(
                    CameraGeometricDefinition{light_position,
                                              light_position + light_forward,
                                              scene_light.get_up()},
                    90.0f, 1.0f,
                    CameraPlanes{0.1f, 100.0},
                    CameraMode::Perspective);

            const auto light_view_matrix = light_camera.get_view_matrix();
            const auto light_projection_matrix = light_camera.get_projection_matrix();
            std::vector<glm::mat4> light_transformations;
            light_transformations.reserve(6);
            for (auto i = 0u; i < 6; ++i) {
                light_transformations.push_back(light_projection_matrix * glm::lookAt(
                        light_position,
                        light_position +
                        OpenGL3_Cubemap::directions[i],
                        OpenGL3_Cubemap::ups[i]));
            }


            glm::mat4 ies_light_model_matrix = glm::identity<glm::mat4>();
            ies_light_model_matrix = glm::translate(ies_light_model_matrix, light_position);
            ies_light_model_matrix = ies_light_model_matrix * glm::mat4_cast(scene_light.get_orientation());
            ies_light_model_matrix = glm::rotate(ies_light_model_matrix, glm::radians(90.0f),
                                                 glm::vec3(1.0f, 0.0f, 0.0f));
            ies_light_model_matrix = glm::scale(ies_light_model_matrix, glm::vec3(scale_modifier));

            glm::mat4 ies_light_inverse_transposed = glm::transpose(glm::inverse(ies_light_model_matrix));

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            mask_fbo->bind_as(GL_FRAMEBUFFER);
            glViewport(0, 0, texture_dimension[0], texture_dimension[1]);
            OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glCullFace(GL_FRONT);
            depthmask_shader->use();
            for (unsigned int i = 0u; i < 6; ++i) {
                depthmask_shader->set_mat4(light_transforms_strings[i],
                                           light_transformations[i]);
            }
            depthmask_shader->set_mat4("model", ies_light_model_matrix);
            depthmask_shader->set_mat4("inversed_transposed_model", ies_light_inverse_transposed);
            depthmask_shader->set_vec3("light_position", light_position);
            depthmask_shader->set_float("far_plane", 100.0f);
            depthmask_shader->set_float("furthest_distance", largest_position_component * scale_modifier);
            OpenGL3_Renderer::draw(ies_light_vao);
            mask_fbo->unbind_from(GL_FRAMEBUFFER);
            glCullFace(GL_BACK);

            rsm_fbo->bind_as(GL_FRAMEBUFFER);
            glViewport(0, 0, texture_dimension[0], texture_dimension[1]);
            OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //  RSM Shader uniforms
            rsm_generation_shader->use();
            rsm_generation_shader->set_float("light_intensity", light_intensity);
            rsm_generation_shader->set_mat4("light_projection", light_projection_matrix);
            rsm_generation_shader->set_float("far_plane", 2000.0f);
            set_light_in_shader(scene_light, rsm_generation_shader);

            rsm_generation_shader->set_bool("is_masking", ies_masking);
            rsm_generation_shader->set_int("ies_mask", 0);
            bind_texture_in_slot(0, ies_light_mask.get());

            for (unsigned int i = 0u; i < 6; ++i) {
                rsm_generation_shader->set_mat4(light_transforms_strings[i],
                                                light_transformations[i]);
            }

            if (!scene_objects.empty()) {
                for (const auto& drawable : scene_objects) {
                    const auto& model_matrix = drawable.transform;
                    rsm_generation_shader->set_mat4("model", model_matrix);
                    rsm_generation_shader->set_vec4("diffuse_color", drawable.material.diffuse_color);
                    OpenGL3_Renderer::draw(*(drawable.vao));
                }
            }

            rsm_fbo->unbind_from(GL_FRAMEBUFFER);

            glViewport(0, 0, viewport_dimension[2], viewport_dimension[3]);
            OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
            OpenGL3_Renderer::clear();

            if (draw_indirect_light) {
                draw_shader->use();
                draw_shader->set_bool("hide_direct_component", hide_direct_component);
            }

            draw_indirect_light ? draw_scene(existing_camera, light_view_matrix, light_projection_matrix,
                                             draw_shader)
                                : draw_scene(existing_camera, light_view_matrix, light_projection_matrix,
                                             no_indirect_shader);

            if (ies_light_wireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                wireframe_shader->use();
                wireframe_shader->set_mat4("view", existing_camera->view_matrix());
                wireframe_shader->set_mat4("projection", existing_camera->projection_matrix());
                wireframe_shader->set_mat4("model", ies_light_model_matrix);
                wireframe_shader->set_mat4("transpose_inverse_model", ies_light_inverse_transposed);
                wireframe_shader->set_vec4("wireframe_color", wireframe_color);
                OpenGL3_Renderer::draw(ies_light_vao);
            }
        }
    }

    void SceneLayer::on_imgui_render() {
        auto light_position = scene_light.get_position_as_vec3();
        auto light_angles = scene_light.get_rotation_in_degrees();
        ImGui::Begin("Shader controls");
        ImGui::Text("Light transform");
        if (ImGui::DragFloat3("Light position", glm::value_ptr(light_position), 0.05f, -5.0f, 5.0f, "%.3f")) {
            scene_light.translate_to(glm::vec4(light_position, 1.0f));
        }
        if (ImGui::DragFloat3("Light rotation angle", glm::value_ptr(light_angles), 1.0f, 0.0f, 360.0f, "%.3f")) {
            scene_light.set_rotation(light_angles);
        }

        ImGui::SliderFloat("Light intensity", &light_intensity, 0.1f, 20.0f);
        ImGui::Spacing();
        ImGui::SliderFloat("Light attenuation (constant factor)", &scene_light.attenuation.constant, 0.5f, 1.5f);
        ImGui::SliderFloat("Light attenuation (linear factor)", &scene_light.attenuation.linear, 0.01f, 2.0f, "%.5f",
                           ImGuiSliderFlags_Logarithmic);
        ImGui::SliderFloat("Light attenuation (quadratic factor)", &scene_light.attenuation.quadratic, 0.01f, 2.0f,
                           "%.6f", ImGuiSliderFlags_Logarithmic);
        ImGui::Spacing();
        if (draw_indirect_light) {
            ImGui::SliderFloat("Indirect Component Intensity", &indirect_intensity, 1.0f, 1000.0f, "%.3f",
                               ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat("Max radius sample", &max_radius, 0.0001f, 1.0f, "%.3f");
            ImGui::Checkbox("Visualize only indirect lighting", &hide_direct_component);
        }
        ImGui::Spacing();
        ImGui::Checkbox("Visualize IES light wireframe", &ies_light_wireframe);
        ImGui::SliderFloat("Wireframe scaling", &scale_modifier, 0.00001f, 2.0f, "%.5f",
                           ImGuiSliderFlags_Logarithmic);
        ImGui::Text("Max component by scale factor: %.5f", largest_position_component * scale_modifier);
        ImGui::ColorEdit4("Wireframe color", glm::value_ptr(wireframe_color), ImGuiColorEditFlags_NoPicker);
        ImGui::Spacing();
        ImGui::Checkbox("Use IES light wireframe to mask light emission", &ies_masking);
        ImGui::End();
    }

    bool SceneLayer::set_light_in_shader(const Spotlight& light, std::shared_ptr<Shader>& shader) {
        shader->set_vec3("scene_light.position", light.get_position_as_vec3());
        shader->set_vec3("scene_light.direction", light.get_forward());
        shader->set_float("scene_light.cutoff_angle", light.spot_params.cosine_cutoff_angle);
        shader->set_float("scene_light.outer_cutoff_angle", light.spot_params.cosine_outer_cutoff_angle);
        shader->set_float("scene_light.constant_attenuation", light.attenuation.constant);
        shader->set_float("scene_light.linear_attenuation", light.attenuation.linear);
        shader->set_float("scene_light.quadratic_attenuation", light.attenuation.quadratic);
        return glGetError() != 0;
    }

    bool SceneLayer::set_light_in_shader(const Point_Light& light, std::shared_ptr<Shader>& shader) {
        shader->set_vec3("scene_light.position", light.get_position_as_vec3());
        shader->set_vec3("scene_light.direction", light.get_forward());
        shader->set_float("scene_light.constant_attenuation", light.attenuation.constant);
        shader->set_float("scene_light.linear_attenuation", light.attenuation.linear);
        shader->set_float("scene_light.quadratic_attenuation", light.attenuation.quadratic);
        return glGetError() != 0;
    }

    void SceneLayer::bind_texture_in_slot(const unsigned int slot_number, OpenGL3_Texture1D* texture) {
        texture->make_active_in_slot(slot_number);
        glBindTexture(texture->bound_type, texture->id);
    }

    void SceneLayer::bind_texture_in_slot(const unsigned int slot_number, OpenGL3_Texture2D* texture) {
        texture->make_active_in_slot(slot_number);
        glBindTexture(texture->bound_type, texture->id);
    }

    void SceneLayer::bind_texture_in_slot(const unsigned int slot_number, OpenGL3_Cubemap* texture) {
        texture->make_active_in_slot(slot_number);
        glBindTexture(texture->bound_type, texture->id);
    }

    void SceneLayer::draw_scene(const std::shared_ptr<FlyCamera>& view_camera,
                                const glm::mat4& light_view_matrix,
                                const glm::mat4& light_projection_matrix,
                                std::shared_ptr<Shader>& shader) {

        //  Shader uniforms
        shader->use();
        shader->set_mat4("light_view", light_view_matrix);
        shader->set_mat4("light_projection", light_projection_matrix);
        shader->set_float("far_plane", 2000.0f);
        shader->set_mat4("view", view_camera->view_matrix());
        shader->set_mat4("projection", view_camera->projection_matrix());
        shader->set_vec3("camera_position", view_camera->position());
        set_light_in_shader(scene_light, shader);
        shader->set_bool("ies_masking", ies_masking);

        //  Texture location binding
        shader->set_int("shadow_map", 0);
        shader->set_int("position_map", 1);
        shader->set_int("normal_map", 2);
        shader->set_int("flux_map", 3);
        shader->set_int("ies_mask", 4);
        shader->set_int("sample_array", 5);
        shader->set_int("samples_number", samples_number);
        bind_texture_in_slot(0, depth_texture.get());
        bind_texture_in_slot(1, position_texture.get());
        bind_texture_in_slot(2, normal_texture.get());
        bind_texture_in_slot(3, flux_texture.get());
        bind_texture_in_slot(4, ies_light_mask.get());
        bind_texture_in_slot(5, samples_texture.get());

        //  Tweakable values
        shader->set_float("light_intensity", light_intensity);
        shader->set_float("indirect_intensity", indirect_intensity);
        shader->set_float("max_radius", max_radius);
        shader->set_float("shadow_threshold", shadow_threshold);
        shader->set_float("furthest_photometric_distance", largest_position_component * scale_modifier);

        if (!scene_objects.empty()) {
            for (const auto& drawable : scene_objects) {
                drawable.material.bind_uniforms_to(shader);
                const auto& model_matrix = drawable.transform;
                const auto& transpose_inverse_matrix = drawable.transpose_inverse_transform;
                shader->set_mat4("model", model_matrix);
                shader->set_mat4("transpose_inverse_model", transpose_inverse_matrix);
                OpenGL3_Renderer::draw(*(drawable.vao));
            }
        }
    }

    bool SceneLayer::on_key_pressed(KeyPressedEvent event) {
        if (event.get_keycode() == GLFW_KEY_F1) {
            draw_indirect_light = !draw_indirect_light;
        }
        return false;
    }
}


