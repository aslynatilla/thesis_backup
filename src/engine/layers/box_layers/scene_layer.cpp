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

            angel = scenes::load_scene_objects_from("resources/Winged_Victory.obj",
                                                    ai_postprocess_flags);

            //  This scaling is needed for the cornell_box_multimaterial.obj scene
            //  The scene has a maximum height of 548.0f; to take it in the range [0, 3] we divide by:
            //  548.0f / 3.0f ~= 185.0f
            const auto scaling_factor = 1.0f / 185.0f;
            const auto scale_by = [](const float scale_factor, std::vector<SceneObject>& scene) {
                for (auto&& object : scene) {
                    const auto T = object.transform;
                    const auto scene_scaling = glm::scale(T, glm::vec3(scale_factor));
                    const auto transposed_inverse_scene_scaling = glm::transpose(glm::inverse(scene_scaling));
                    object.transform = scene_scaling;
                    object.transpose_inverse_transform = transposed_inverse_scene_scaling;
                }
            };
            scale_by(scaling_factor, scene_objects);
            for (auto&& object : angel) {
                const auto T = object.transform;
                object.transform = glm::translate(T, glm::vec3(1.0f, 0.85f, 1.0f));
                const auto transposed_inverse = glm::transpose(glm::inverse(T));
                object.transpose_inverse_transform = transposed_inverse;
            }
            scale_by(1.0f / 260.0f, angel);

            scene_light = Point_Light(glm::vec4(1.5f, 2.6f, 1.5f, 1.0f),
                                      LightAttenuationParameters{1.0f, 0.5f, 1.8f});
            scene_light.set_rotation(glm::vec3(90.0f, 0.0f, 0.0f));

            draw_shader = shader::create_shader_from("resources/shaders/rsm_render.vert",
                                                     "resources/shaders/rsm_render.frag");
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
                    {static_cast<int>(viewport_dimension[2] / 2),
                     static_cast<int>(viewport_dimension[3]) / 2};

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
            fake_default_fbo = std::make_unique<OpenGL3_FrameBuffer>();
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

            fake_texture = std::make_unique<OpenGL3_Texture2D>(GL_SRGB8,
                                                               OpenGL3_TextureParameters(
                                                                       {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                        GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                       {GL_NEAREST, GL_NEAREST,
                                                                        GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                                        viewport_dimension[2],
                                                                        viewport_dimension[3],
                                                                        GL_RGB, GL_FLOAT, nullptr);

            fake_depth = std::make_unique<OpenGL3_Texture2D>(GL_DEPTH_COMPONENT,
                                                             OpenGL3_TextureParameters(
                                                                     {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                      GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                     {GL_LINEAR, GL_LINEAR,
                                                                      GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                                        viewport_dimension[2],
                                                                        viewport_dimension[3],
                                                             GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);


            VPL_samples_per_fragment = 200;
            const auto samples = random_num::uniform_samples_on_unit_sphere(VPL_samples_per_fragment);

            //  TODO: consider using BufferTexture for this
            //  see: https://www.khronos.org/opengl/wiki/Buffer_Texture
            samples_texture = std::make_unique<OpenGL3_Texture1D>(GL_RGB32F,
                                                                  OpenGL3_TextureParameters(
                                                                          {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                           GL_TEXTURE_WRAP_S},
                                                                          {GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE}),
                                                                  VPL_samples_per_fragment,
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
            fake_default_fbo->bind_as(GL_FRAMEBUFFER);
            fake_default_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *fake_depth);
            fake_default_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *fake_texture);
            glDrawBuffers(1, buffer_enums.get());
            fake_default_fbo->unbind_from();


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
            ies::adapter::IES_Mesh photometric_solid = ies::adapter::IES_Mesh::interpolate_from(document, 3);

            const auto vertices = photometric_solid.get_vertices();

            const auto farthest_vertex_distance = [](const std::vector<float>& vs) -> float {
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


            //   16 bytes per matrix column, times 4 because they are mat4; times 4
            //  again because we got 4 matrices
            matrices_buffer = std::make_shared<UniformBuffer>((4 * 4 * 4) * 4);
            matrices_buffer->bind_to_binding_point(0);
            matrices_buffer->unbind_from_uniform_buffer_target();

            //   16 is the number of bytes per float times the number of floats in a vec4
            //   4 is the number of bytes of a float, and we need to use 3 floats, rounded to 4
            //  for the rules implied by std140 alignment
            light_data_buffer = std::make_shared<UniformBuffer>((16 * 3) + (4 * 4));
            light_data_buffer->bind_to_binding_point(1);
            light_data_buffer->unbind_from_uniform_buffer_target();

            //   5 colors and 3 floats (rounded to 4)
            material_buffer = std::make_shared<UniformBuffer>((16 * 5) + (4 * 4));
            material_buffer->bind_to_binding_point(2);
            material_buffer->unbind_from_uniform_buffer_target();

            //  2 floats and 1 bool
            common_data_buffer = std::make_shared<UniformBuffer>(4 * 3);
            common_data_buffer->bind_to_binding_point(3);
            material_buffer->unbind_from_uniform_buffer_target();
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

    void SceneLayer::update([[maybe_unused]] float delta_time) {
        if (auto existing_camera = view_camera.lock()) {
            //  Setup and compute common data
            const auto camera_view_matrix = existing_camera->view_matrix();
            const auto camera_projection_matrix = existing_camera->projection_matrix();
            const auto light_data = scene_light.get_representative_data();
            const auto light_position = glm::vec3(light_data.position);
            const auto light_orientation = glm::mat4_cast(scene_light.get_orientation());
            const auto light_camera = Camera(
                    CameraGeometricDefinition{light_data.position,
                                              light_data.position + light_data.direction,
                                              scene_light.get_up()},
                    90.0f, 1.0f,
                    CameraPlanes{0.1f, light_far_plane},
                    CameraMode::Perspective);

            const auto light_projection_matrix = light_camera.get_projection_matrix();
            const auto light_transformations = compute_VP_transform_for_cubemap(light_position,
                                                                                light_projection_matrix);

            const auto ies_light_model_matrix = compute_light_model_matrix(light_position,
                                                                           light_orientation,
                                                                           scale_modifier);

            const glm::mat4 ies_light_inverse_transposed = glm::transpose(glm::inverse(ies_light_model_matrix));
            light_data_buffer->bind_to_uniform_buffer_target();
            light_data_buffer->copy_to_buffer(0, 44, light_data.raw());
            light_data_buffer->copy_to_buffer(44, 4, &light_intensity);
            light_data_buffer->copy_to_buffer(48, 16, glm::value_ptr(light_color));
            light_data_buffer->unbind_from_uniform_buffer_target();

            const float distance_to_furthest_ies_vertex = largest_position_component * scale_modifier;
            common_data_buffer->bind_to_uniform_buffer_target();
            common_data_buffer->copy_to_buffer(0, 4, &light_far_plane);
            common_data_buffer->copy_to_buffer(4, 4, &distance_to_furthest_ies_vertex);
            common_data_buffer->copy_to_buffer(8, 1, &is_using_ies_masking);
            common_data_buffer->unbind_from_uniform_buffer_target();


            //  Depth mask uniforms and drawing
            mask_fbo->bind_as(GL_FRAMEBUFFER);
            glCullFace(GL_FRONT);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glViewport(0, 0, texture_dimension[0], texture_dimension[1]);
            OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            depthmask_shader->use();
            for (unsigned int i = 0u; i < 6; ++i) {
                depthmask_shader->set_mat4(0 + i,
                                           light_transformations[i]);
            }

            matrices_buffer->bind_to_uniform_buffer_target();
            matrices_buffer->copy_to_buffer(0, 4 * 4 * 4, glm::value_ptr(ies_light_model_matrix));
            matrices_buffer->copy_to_buffer(64, 4 * 4 * 4, glm::value_ptr(ies_light_inverse_transposed));
            matrices_buffer->unbind_from_uniform_buffer_target();
            OpenGL3_Renderer::draw(ies_light_vao);
            mask_fbo->unbind_from(GL_FRAMEBUFFER);


            //  RSM uniforms and drawing
            rsm_fbo->bind_as(GL_FRAMEBUFFER);
            glCullFace(GL_BACK);
            glViewport(0, 0, texture_dimension[0], texture_dimension[1]);
            OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            rsm_generation_shader->use();

            for (unsigned int i = 0u; i < 6; ++i) {
                rsm_generation_shader->set_mat4(0 + i,
                                                light_transformations[i]);
            }

            rsm_generation_shader->set_int(7, 0);
            ies_light_mask->bind_to_slot(0);

            if (!scene_objects.empty()) {
                for (const auto& drawable : scene_objects) {
                    const auto& model_matrix = drawable.transform;
                    const auto& inverse_model_matrix = drawable.transpose_inverse_transform;
                    matrices_buffer->bind_to_uniform_buffer_target();
                    matrices_buffer->copy_to_buffer(0, 4 * 4 * 4, glm::value_ptr(model_matrix));
                    matrices_buffer->copy_to_buffer(64, 4 * 4 * 4, glm::value_ptr(inverse_model_matrix));
                    matrices_buffer->unbind_from_uniform_buffer_target();
                    rsm_generation_shader->set_vec4(6, drawable.material.data.diffuse_color);
                    OpenGL3_Renderer::draw(*(drawable.vao));
                }
            }

            if (!angel.empty()) {
                for (const auto& drawable : angel) {
                    const auto& model_matrix = drawable.transform;
                    const auto& inverse_model_matrix = drawable.transpose_inverse_transform;
                    matrices_buffer->bind_to_uniform_buffer_target();
                    matrices_buffer->copy_to_buffer(0, 4 * 4 * 4, glm::value_ptr(model_matrix));
                    matrices_buffer->copy_to_buffer(64, 4 * 4 * 4, glm::value_ptr(inverse_model_matrix));
                    matrices_buffer->unbind_from_uniform_buffer_target();
                    rsm_generation_shader->set_vec4(6, drawable.material.data.diffuse_color);
                    OpenGL3_Renderer::draw(*(drawable.vao));
                }
            }
            rsm_fbo->unbind_from(GL_FRAMEBUFFER);

            fake_default_fbo->bind_as(GL_FRAMEBUFFER);
            //  Final pass for rendering the scene
            glViewport(0, 0, viewport_dimension[2], viewport_dimension[3]);
            OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
            OpenGL3_Renderer::clear();

            if (draw_indirect_light) {
                draw_shader->use();
                draw_shader->set_bool(11, hide_direct_component);
            }

            glEnable(GL_FRAMEBUFFER_SRGB);
            draw_indirect_light ? draw_scene(glm::vec3(), camera_view_matrix, camera_projection_matrix, draw_shader)
                                : draw_scene(
                    glm::vec3(), camera_view_matrix, camera_projection_matrix, no_indirect_shader);
            glDisable(GL_FRAMEBUFFER_SRGB);
            fake_default_fbo->unbind_from();
            
            if (ies_light_wireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                wireframe_shader->use();
                matrices_buffer->bind_to_uniform_buffer_target();
                matrices_buffer->copy_to_buffer(0, 4 * 4 * 4, glm::value_ptr(ies_light_model_matrix));
                matrices_buffer->copy_to_buffer(64, 4 * 4 * 4, glm::value_ptr(ies_light_inverse_transposed));
                matrices_buffer->copy_to_buffer(128, 4 * 4 * 4, glm::value_ptr(camera_view_matrix));
                matrices_buffer->copy_to_buffer(192, 4 * 4 * 4, glm::value_ptr(camera_projection_matrix));
                matrices_buffer->unbind_from_uniform_buffer_target();
                wireframe_shader->set_vec4(4, wireframe_color);
                OpenGL3_Renderer::draw(ies_light_vao);
            }
        }
    }

    std::vector<glm::mat4>
    SceneLayer::compute_VP_transform_for_cubemap(const glm::vec3 view_position,
                                                 const glm::mat4 projection_matrix) const {
        std::vector<glm::mat4> VP_transformation;
        VP_transformation.reserve(6);
        for (auto i = 0u; i < 6; ++i) {
            VP_transformation.push_back(projection_matrix * glm::lookAt(
                    view_position,
                    view_position +
                    OpenGL3_Cubemap::directions[i],
                    OpenGL3_Cubemap::ups[i]));
        }
        return VP_transformation;
    }

    glm::mat4 SceneLayer::compute_light_model_matrix(const glm::vec3 position, const glm::mat4 orientation_matrix,
                                                     const float scale) const {
        auto model = glm::identity<glm::mat4>();
        model = glm::translate(model, position);
        model = model * orientation_matrix;
        model = glm::rotate(model, glm::radians(90.0f),
                            glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(scale));
        return model;
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
            ImGui::SliderFloat("Displacing sphere scale", &displacement_sphere_radius, 0.0001f, 2.0f, "%.3f");
            ImGui::Checkbox("Visualize only indirect lighting", &hide_direct_component);
        }
        ImGui::Spacing();
        ImGui::Checkbox("Visualize IES light wireframe", &ies_light_wireframe);
        ImGui::SliderFloat("Wireframe scaling", &scale_modifier, 0.00001f, 2.0f, "%.5f",
                           ImGuiSliderFlags_Logarithmic);
        ImGui::Text("Max component by scale factor: %.5f", largest_position_component * scale_modifier);
        ImGui::ColorEdit4("Wireframe color", glm::value_ptr(wireframe_color), ImGuiColorEditFlags_NoPicker);
        ImGui::Spacing();
        ImGui::Checkbox("Use IES light wireframe to mask light emission", &is_using_ies_masking);
        ImGui::End();
    }

    void SceneLayer::draw_scene(const glm::vec3 camera_position, const glm::mat4 view_matrix,
                                const glm::mat4 projection_matrix, std::shared_ptr<Shader>& shader) {

        //  Shader uniforms
        shader->use();
        matrices_buffer->bind_to_uniform_buffer_target();
        matrices_buffer->copy_to_buffer(128, 4 * 4 * 4, glm::value_ptr(view_matrix));
        matrices_buffer->copy_to_buffer(192, 4 * 4 * 4, glm::value_ptr(projection_matrix));
        matrices_buffer->unbind_from_uniform_buffer_target();

        //  Texture location binding
        shader->set_int(0, 0);
        depth_texture->bind_to_slot(0);
        shader->set_int(1, 1);
        position_texture->bind_to_slot(1);
        shader->set_int(2, 2);
        normal_texture->bind_to_slot(2);
        shader->set_int(3, 3);
        flux_texture->bind_to_slot(3);
        shader->set_int(4, 4);
        ies_light_mask->bind_to_slot(4);
        shader->set_int(5, 5);
        samples_texture->bind_to_slot(5);

        //  Tweakable values and common uniforms
        shader->set_int(6, VPL_samples_per_fragment);
        shader->set_vec3(7, camera_position);
        shader->set_float(8, shadow_threshold);
        shader->set_float(9, displacement_sphere_radius);
        shader->set_float(10, indirect_intensity);

        if (!scene_objects.empty()) {
            for (const auto& drawable : scene_objects) {
                const auto& model_matrix = drawable.transform;
                const auto& transpose_inverse_matrix = drawable.transpose_inverse_transform;
                matrices_buffer->bind_to_uniform_buffer_target();
                matrices_buffer->copy_to_buffer(0, 4 * 4 * 4, glm::value_ptr(model_matrix));
                matrices_buffer->copy_to_buffer(64, 4 * 4 * 4, glm::value_ptr(transpose_inverse_matrix));
                matrices_buffer->unbind_from_uniform_buffer_target();

                material_buffer->bind_to_uniform_buffer_target();
                material_buffer->copy_to_buffer(0, sizeof(MaterialData), drawable.material.data.raw());
                material_buffer->unbind_from_uniform_buffer_target();
                OpenGL3_Renderer::draw(*(drawable.vao));
            }
        }

        if (!angel.empty()) {
            for (const auto& drawable : angel) {
                const auto& model_matrix = drawable.transform;
                const auto& transpose_inverse_matrix = drawable.transpose_inverse_transform;
                matrices_buffer->bind_to_uniform_buffer_target();
                matrices_buffer->copy_to_buffer(0, 4 * 4 * 4, glm::value_ptr(model_matrix));
                matrices_buffer->copy_to_buffer(64, 4 * 4 * 4, glm::value_ptr(transpose_inverse_matrix));
                matrices_buffer->unbind_from_uniform_buffer_target();

                material_buffer->bind_to_uniform_buffer_target();
                material_buffer->copy_to_buffer(0, sizeof(MaterialData), drawable.material.data.raw());
                material_buffer->unbind_from_uniform_buffer_target();
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


