#include "scene_layer.h"

namespace engine {

    SceneLayer::SceneLayer(std::weak_ptr<FlyCamera> application_camera)
    : view_camera(std::move(application_camera))
    {}

    void SceneLayer::on_attach() {
        {
            constexpr unsigned int ai_postprocess_flags = aiProcess_GenNormals |
                                                          aiProcess_Triangulate |
                                                          aiProcess_ValidateDataStructure;
            scene_objects = scenes::load_scene_objects_from("resources/cornell_box_multimaterial.obj",
                                                            ai_postprocess_flags);

            scene_light = SpotLight(glm::vec3(278.0f, 548.0f, 279.5f),
                                    glm::vec3(0.0f, -1.0f, 0.0f),
                                    30.0f, 60.0f,
                                    1.0f, 0.004f, 0.00009f);

            draw_shader = shader::create_shader_from("resources/shaders/shadowmapped.vert",
                                                     "resources/shaders/shadowmapped.frag");
            no_indirect_shader = shader::create_shader_from("resources/shaders/shadowmapped_no_indirect.vert",
                                                            "resources/shaders/shadowmapped_no_indirect.frag");
            rsm_generation_shader = shader::create_shader_from("resources/shaders/rsm.vert",
                                                               "resources/shaders/rsm.frag");
            wireframe_shader = shader::create_shader_from("resources/shaders/wireframe.vert",
                                                          "resources/shaders/wireframe.frag");

            auto viewport_float_dimension = std::make_unique<float[]>(4);
            glGetFloatv(GL_VIEWPORT, viewport_float_dimension.get());
            viewport_dimension[0] = static_cast<unsigned int>(viewport_float_dimension[0]);
            viewport_dimension[1] = static_cast<unsigned int>(viewport_float_dimension[1]);
            viewport_dimension[2] = static_cast<unsigned int>(viewport_float_dimension[2]);
            viewport_dimension[3] = static_cast<unsigned int>(viewport_float_dimension[3]);

            texture_dimension =
                    {static_cast<unsigned int>(viewport_dimension[2] / 2),
                     static_cast<unsigned int>(viewport_dimension[3]) / 2};

            rsm_fbo = std::make_unique<OpenGL3_FrameBuffer>();
            depth_texture = std::make_unique<OpenGL3_Texture2D>(GL_DEPTH_COMPONENT,
                                                                OpenGL3_TextureParameters(
                                                                        {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                         GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                        {GL_LINEAR, GL_LINEAR,
                                                                         GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                                texture_dimension[0],
                                                                texture_dimension[1],
                                                                GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            //    Clamping to border allows "not producing" a shadow
            //  in the case sampling outside of the texture happens
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));

            position_texture = std::make_unique<OpenGL3_Texture2D>(GL_RGB32F,
                                                                   OpenGL3_TextureParameters(
                                                                           {GL_TEXTURE_MIN_FILTER,
                                                                            GL_TEXTURE_MAG_FILTER,
                                                                            GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                           {GL_LINEAR, GL_LINEAR,
                                                                            GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                                   texture_dimension[0],
                                                                   texture_dimension[1],
                                                                   GL_RGB, GL_FLOAT, nullptr);
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)));

            normal_texture = std::make_unique<OpenGL3_Texture2D>(GL_RGB32F,
                                                                 OpenGL3_TextureParameters(
                                                                         {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                          GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                         {GL_LINEAR, GL_LINEAR,
                                                                          GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                                 texture_dimension[0],
                                                                 texture_dimension[1],
                                                                 GL_RGB, GL_FLOAT, nullptr);
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)));
            flux_texture = std::make_unique<OpenGL3_Texture2D>(GL_RGB8,
                                                               OpenGL3_TextureParameters(
                                                                       {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                        GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                                       {GL_LINEAR, GL_LINEAR,
                                                                        GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                               texture_dimension[0],
                                                               texture_dimension[1],
                                                               GL_RGB, GL_FLOAT, nullptr);
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)));

            samples_number = 400;
            const auto samples = random_num::random_polar_offsets(samples_number);

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
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);

            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
//            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);


            const auto path_to_IES_data = files::make_path_absolute("resources/ies/TEST.IES");
            document = parser.parse(path_to_IES_data.filename().string(), files::read_file(path_to_IES_data));
            ies::adapter::IES_Mesh photometric_solid(document);

            const auto vertices = photometric_solid.get_vertices();

            largest_position_component = std::max_element(std::begin(vertices), std::end(vertices)).operator*();

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
        if(auto existing_camera = view_camera.lock()){
            const auto light_camera = Camera(
                    CameraGeometricDefinition{scene_light.position,
                                              scene_light.position + scene_light.direction,
                                              glm::vec3(0.0f, 0.0f, -1.0f)},
                    90.0f, 1.0f,
                    CameraPlanes{0.1f, 2000.0f},
                    CameraMode::Perspective);

            const auto light_view_matrix = light_camera.get_view_matrix();
            const auto light_projection_matrix = light_camera.get_projection_matrix();

            rsm_fbo->bind_as(GL_FRAMEBUFFER);
            glViewport(0, 0, texture_dimension[0], texture_dimension[1]);
            OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //  RSM Shader uniforms
            rsm_generation_shader->use();
            rsm_generation_shader->set_float("light_intensity", light_intensity);
            rsm_generation_shader->set_mat4("light_view", light_view_matrix);
            rsm_generation_shader->set_mat4("light_projection", light_projection_matrix);
            rsm_generation_shader->set_float("far_plane", 2000.0f);
            set_light_in_shader(scene_light, rsm_generation_shader);

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

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            wireframe_shader->use();
            wireframe_shader->set_mat4("view", existing_camera->view_matrix());
            wireframe_shader->set_mat4("projection", existing_camera->projection_matrix());

            glm::mat4 ies_light_model_matrix = glm::identity<glm::mat4>();
            ies_light_model_matrix = glm::translate(ies_light_model_matrix, light_camera.get_position());
            ies_light_model_matrix = glm::rotate(ies_light_model_matrix, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ies_light_model_matrix = glm::scale(ies_light_model_matrix, glm::vec3(scale_modifier));

            glm::mat4 ies_light_inverse_transpose = glm::transpose(glm::inverse(ies_light_model_matrix));
            wireframe_shader->set_mat4("model", ies_light_model_matrix);
            wireframe_shader->set_mat4("transpose_inverse_model", ies_light_inverse_transpose);
            wireframe_shader->set_vec4("wireframe_color", glm::vec4(0.2f, 1.0f, 1.0f, 1.0f));
            OpenGL3_Renderer::draw(ies_light_vao);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    void SceneLayer::on_imgui_render() {
        ImGui::Begin("Shader controls");
        ImGui::SliderFloat("Spotlight Intensity", &light_intensity, 0.5f, 15.0f);
        if (draw_indirect_light) {
            ImGui::SliderFloat("Indirect Component Intensity", &indirect_intensity, 1.0f, 1000.0f, "%.3f",
                               ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat("Max radius sample", &max_radius, 0.001f, 1.0f, "%.3f");
            ImGui::Checkbox("Visualize only indirect lighting", &hide_direct_component);
        }
        ImGui::SliderFloat("Scaling dimension", &scale_modifier, 0.001f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
        ImGui::End();
    }

    bool SceneLayer::set_light_in_shader(const SpotLight& light, std::shared_ptr<Shader>& shader) {
        shader->set_vec3("scene_light.position", light.position);
        shader->set_vec3("scene_light.direction", light.direction);
        shader->set_float("scene_light.cutoff_angle", light.cosine_cutoff_angle);
        shader->set_float("scene_light.outer_cutoff_angle", light.cosine_outer_cutoff_angle);
        shader->set_float("scene_light.constant_attenuation", light.constant_attenuation_factor);
        shader->set_float("scene_light.linear_attenuation", light.linear_attenuation_factor);
        shader->set_float("scene_light.quadratic_attenuation", light.quadratic_attenuation_factor);
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

        //  Texture location binding
        shader->set_int("shadow_map", 0);
        shader->set_int("position_map", 1);
        shader->set_int("normal_map", 2);
        shader->set_int("flux_map", 3);
        shader->set_int("sample_array", 4);
        shader->set_int("samples_number", samples_number);
        bind_texture_in_slot(0, depth_texture.get());
        bind_texture_in_slot(1, position_texture.get());
        bind_texture_in_slot(2, normal_texture.get());
        bind_texture_in_slot(3, flux_texture.get());
        bind_texture_in_slot(4, samples_texture.get());

        //  Tweakable values
        shader->set_float("light_intensity", light_intensity);
        shader->set_float("indirect_intensity", indirect_intensity);
        shader->set_float("max_radius", max_radius);
        shader->set_float("shadow_threshold", shadow_threshold);

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


