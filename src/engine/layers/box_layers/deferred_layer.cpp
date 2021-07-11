#include "deferred_layer.h"

namespace engine {
    DeferredLayer::DeferredLayer(std::weak_ptr<FlyCamera> controlled_camera, [[maybe_unused]] LayerCreationKey key)
            : camera(std::move(controlled_camera)) {}

    void DeferredLayer::on_attach() {
        constexpr unsigned int postprocessing_flags = aiProcess_GenNormals |
                                                      aiProcess_Triangulate |
                                                      aiProcess_ValidateDataStructure;
        const auto path_to_sponza = files::make_path_absolute("resources/sponza/small_sponza.obj");
        scene_data = scenes::SceneLoader::load_scene_from(path_to_sponza, postprocessing_flags);

        lights.reserve(number_of_lights);
        lights.emplace_back(glm::vec4(0.0f, 0.2f, 0.0f, 1.0f),
                             LightAttenuationParameters{1.0f, 0.5f, 1.8f});
        lights[0].set_rotation(glm::vec3(0.0f, 0.0f, 0.0f));
        lights.emplace_back(glm::vec4(0.0f, 2.5f, 0.0f, 1.0f),
                            LightAttenuationParameters{1.0f, 0.5f, 1.8f});
        lights[1].set_rotation(glm::vec3(90.0f, 0.0f, 0.0f));

        auto viewport_size = std::make_unique<float[]>(4);
        glGetFloatv(GL_VIEWPORT, viewport_size.get());
        std::transform(&viewport_size[2], &viewport_size[4], glm::value_ptr(target_resolution),
                       [](const auto f) { return static_cast<int>(f); });
        texture_resolution = glm::vec2{target_resolution[0] / 2, target_resolution[1] / 2};

        std::array<GLenum, 3> color_attachments{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};

        temp_depth_buffer = OpenGL3_Texture2D_Builder()
                .with_size(target_resolution[0], target_resolution[1])
                .with_texture_format(GL_DEPTH_COMPONENT)
                .with_data_format(GL_DEPTH_COMPONENT)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_minification()
                .using_linear_magnification()
                .using_clamping_to_borders()
                .using_border_color({1.0f, 1.0f, 1.0f, 1.0f})
                .as_resource();

        gbuffer_creation_setup(color_attachments);
        debug_setup();
        ies_mask_creation_setup(color_attachments);     // ies_mask_creation_setup generates a texture that
        rsm_creation_setup(color_attachments);          // rsm_creation_setup uses to generate the RSM
        direct_pass_setup();
        indirect_pass_setup();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glDepthMask(GL_TRUE);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        gbuffer_creation = shader::create_shader_from("resources/shaders/deferred/gbuffer_creation.vert",
                                                      "resources/shaders/deferred/gbuffer_creation.frag");
        wireframe_drawer = shader::create_shader_from("resources/shaders/deferred/gbuffer_creation.vert",
                                                      "resources/shaders/deferred/wireframe.frag");
        quad_render = shader::create_shader_from("resources/shaders/deferred/quad_rendering.vert",
                                                 "resources/shaders/deferred/quad_rendering.frag");
        deferred_direct = shader::create_shader_from("resources/shaders/deferred/quad_rendering.vert",
                                                     "resources/shaders/deferred/deferred_direct.frag");
        rsm_creation = shader::create_shader_from("resources/shaders/deferred/rsm_creation.vert",
                                                  "resources/shaders/deferred/rsm_creation.frag",
                                                  "resources/shaders/deferred/rsm_creation.geom");
        mask_creation = shader::create_shader_from("resources/shaders/deferred/rsm_creation.vert",
                                                   "resources/shaders/deferred/ies_mask_creation.frag",
                                                   "resources/shaders/deferred/rsm_creation.geom");
        deferred_indirect = shader::create_shader_from("resources/shaders/deferred/quad_rendering.vert",
                                                       "resources/shaders/deferred/deferred_indirect.frag");

        const auto path_to_IES_light_1 = files::make_path_absolute("resources/ies/111621PN.IES");
        const auto path_to_IES_light_2 = files::make_path_absolute("resources/ies/ITL53278.ies");
        ies_light_vaos.push_back(std::make_unique<VertexArray>());
        ies_light_vaos.push_back(std::make_unique<VertexArray>());
        load_IES_light_as_VAO(path_to_IES_light_1, 0);
        load_IES_light_as_VAO(path_to_IES_light_2, 1);
        uniform_buffers_setup();
    }


    void DeferredLayer::on_detach() {}

    void DeferredLayer::on_event(Event& event) {
        EventHandler handler(event);
        handler.handle<CameraMovedEvent>([this]([[maybe_unused]] auto&& ...args) -> decltype(auto) {
            camera_moved = true;
            return false;
        });

        handler.handle<SceneChangedEvent>([this]([[maybe_unused]] auto&& ...args) -> decltype(auto) {
            scene_changed = true;
            return false;
        });
        event.handled = false;
    }

    void DeferredLayer::update(float delta_time) {
        [[maybe_unused]] float timestep = delta_time;
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);

        if (scene_changed) {
            update_camera_related_buffers();
            update_scene_buffers_and_representations();
            create_gbuffer();
            //  conditionally, draw_wireframe() //TODO: extract function
            if (draw_wireframe_in_scene) {
                debug_fbo->bind_as(GL_FRAMEBUFFER);
                glViewport(0, 0, target_resolution[0], target_resolution[1]);
                OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
                OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT);
                wireframe_drawer->use();
                wireframe_overlay_output->bind_to_slot(2);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                gbuffer_transformation->bind_to_uniform_buffer_target();
                gbuffer_transformation->copy_to_buffer(64, 4 * 4 * 4, glm::value_ptr(ies_model_matrices[0]));
                gbuffer_transformation->copy_to_buffer(128, 4 * 4 * 4,
                                                       glm::value_ptr(ies_inverse_transposed_matrices[0]));
                gbuffer_transformation->unbind_from_uniform_buffer_target();
                material_buffer->bind_to_uniform_buffer_target();
                material_buffer->copy_to_buffer(0, 16, glm::value_ptr(wireframe_color));
                material_buffer->unbind_from_uniform_buffer_target();
                OpenGL3_Renderer::draw(*ies_light_vaos[0]);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                debug_fbo->unbind_from(GL_FRAMEBUFFER);
            }
            scene_changed = false;
            camera_moved = false;
        } else if (camera_moved) {
            update_camera_related_buffers();
            create_gbuffer();
            if (draw_wireframe_in_scene) {
                debug_fbo->bind_as(GL_FRAMEBUFFER);
                glViewport(0, 0, target_resolution[0], target_resolution[1]);
                OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
                OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT);
                wireframe_drawer->use();
                wireframe_overlay_output->bind_to_slot(2);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                gbuffer_transformation->bind_to_uniform_buffer_target();
                gbuffer_transformation->copy_to_buffer(64, 4 * 4 * 4, glm::value_ptr(ies_model_matrices[0]));
                gbuffer_transformation->copy_to_buffer(128, 4 * 4 * 4,
                                                       glm::value_ptr(ies_inverse_transposed_matrices[0]));
                gbuffer_transformation->unbind_from_uniform_buffer_target();
                material_buffer->bind_to_uniform_buffer_target();
                material_buffer->copy_to_buffer(0, 16, glm::value_ptr(wireframe_color));
                material_buffer->unbind_from_uniform_buffer_target();
                OpenGL3_Renderer::draw(*ies_light_vaos[0]);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                debug_fbo->unbind_from(GL_FRAMEBUFFER);
            }
            camera_moved = false;
        }

        render_direct_lighting();
        render_indirect_lighting();

        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);
        glDisable(GL_DEPTH_TEST);

        glEnable(GL_FRAMEBUFFER_SRGB);
        sum_lighting_components();
        glDisable(GL_FRAMEBUFFER_SRGB);
    }

    void DeferredLayer::create_gbuffer() {
        gbuffer_creation_fbo->bind_as(GL_FRAMEBUFFER);
        glViewport(0, 0, target_resolution[0], target_resolution[1]);
        OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gbuffer_creation->use();

        gbuffer_positions_texture->bind_to_slot(0);
        gbuffer_normals_texture->bind_to_slot(1);
        gbuffer_diffuse_texture->bind_to_slot(2);

        gbuffer_creation->set_int(0, 3);
        for (const auto& o : scene_data.objects) {
            const auto& model_matrix = o.transform;
            const auto& transposed_inversed_model_matrix = o.transpose_inverse_transform;
            gbuffer_transformation->bind_to_uniform_buffer_target();
            gbuffer_transformation->copy_to_buffer(64, 64, glm::value_ptr(model_matrix));
            gbuffer_transformation->copy_to_buffer(128, 64, glm::value_ptr(transposed_inversed_model_matrix));
            gbuffer_transformation->unbind_from_uniform_buffer_target();
            material_buffer->bind_to_uniform_buffer_target();
            material_buffer->copy_to_buffer(0, 16, glm::value_ptr(o.material.data.diffuse_color));
            material_buffer->copy_to_buffer(16, 4, &o.material.data.shininess);
            material_buffer->unbind_from_uniform_buffer_target();

            if(o.texture_index >= 0){
                scene_data.textures[o.texture_index]->bind_to_slot(3);
            }

            OpenGL3_Renderer::draw(*(o.vao));
        }
        gbuffer_creation_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    void DeferredLayer::update_light_mask(const std::vector<glm::mat4>& light_transforms, int light_index) {
        mask_creation_fbo->bind_as(GL_FRAMEBUFFER);
        mask_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *rsm_depth_vec[light_index]);
        mask_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *light_masks[light_index]);
        glViewport(0, 0, texture_resolution[0], texture_resolution[1]);
        glCullFace(GL_FRONT);
        OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mask_creation->use();
        for (int i = 0; i < 6; ++i) {
            mask_creation->set_mat4(0 + i,
                                    light_transforms[i]);
        }
        mask_creation->set_int(6, light_index);
        gbuffer_transformation->bind_to_uniform_buffer_target();
        gbuffer_transformation->copy_to_buffer(64, 64, glm::value_ptr(ies_model_matrices[light_index]));
        gbuffer_transformation->unbind_from_uniform_buffer_target();
        OpenGL3_Renderer::draw(*ies_light_vaos[light_index]);
        mask_creation_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    void
    DeferredLayer::update_rsm(const std::vector<glm::mat4>& light_transformations, int light_index) {
        rsm_creation_fbo->bind_as(GL_FRAMEBUFFER);
        rsm_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *rsm_depth_vec[light_index]);
        rsm_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *rsm_position_vec[light_index]);
        rsm_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, *rsm_normal_vec[light_index]);
        rsm_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, *rsm_flux_vec[light_index]);

        glViewport(0, 0, texture_resolution[0], texture_resolution[1]);
        glCullFace(GL_BACK);
        OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rsm_creation->use();
        rsm_position_vec[light_index]->bind_to_slot(0);
        rsm_normal_vec[light_index]->bind_to_slot(1);
        rsm_flux_vec[light_index]->bind_to_slot(2);
        light_masks[light_index]->bind_to_slot(3);

        for (int i = 0; i < 6; ++i) {
            rsm_creation->set_mat4(0 + i, light_transformations[i]);
        }

        rsm_creation->set_int(6, 3);
        rsm_creation->set_int(7, 4);
        rsm_creation->set_int(8, light_index);
        for (const auto& o : scene_data.objects) {
            const auto& model_matrix = o.transform;
            const auto& transposed_inversed_model_matrix = o.transpose_inverse_transform;
            gbuffer_transformation->bind_to_uniform_buffer_target();
            gbuffer_transformation->copy_to_buffer(64, 64, glm::value_ptr(model_matrix));
            gbuffer_transformation->copy_to_buffer(128, 64, glm::value_ptr(transposed_inversed_model_matrix));
            gbuffer_transformation->unbind_from_uniform_buffer_target();
            material_buffer->bind_to_uniform_buffer_target();
            material_buffer->copy_to_buffer(0, 16, glm::value_ptr(o.material.data.diffuse_color));
            material_buffer->copy_to_buffer(16, 4, &o.material.data.shininess);
            material_buffer->unbind_from_uniform_buffer_target();

            if(o.texture_index >= 0){
                scene_data.textures[o.texture_index]->bind_to_slot(4);
            }
            OpenGL3_Renderer::draw(*(o.vao));
        }
        rsm_creation_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    void DeferredLayer::render_direct_lighting() {
        direct_pass_fbo->bind_as(GL_FRAMEBUFFER);
        glViewport(0, 0, target_resolution[0], target_resolution[1]);
        OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        deferred_direct->use();
        deferred_direct->set_int(0, 0);
        deferred_direct->set_int(1, 1);
        deferred_direct->set_int(2, 2);

        //  Bind RSM depthmaps
        for(int i = 0; i < number_of_lights; ++i){
            deferred_direct->set_int(3 + i, 3 + i);
        }
        //  Bind light IES masks
        for(int i = 0; i < number_of_lights; ++i){
            deferred_direct->set_int(3 + number_of_lights + i, 3 + number_of_lights + i);
        }

//        deferred_direct->set_int(3, 3);
//        deferred_direct->set_int(4, 4);
        gbuffer_positions_texture->bind_to_slot(0);
        gbuffer_normals_texture->bind_to_slot(1);
        gbuffer_diffuse_texture->bind_to_slot(2);

        for(int slot_number = 3; auto&& depthmap : rsm_depth_vec){
            depthmap->bind_to_slot(slot_number);
            ++slot_number;
        }
        for(int slot_number = 3 + number_of_lights; auto&& mask : light_masks){
            mask->bind_to_slot(slot_number);
            ++slot_number;
        }

        OpenGL3_Renderer::draw(quad.vao);
        direct_pass_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    void DeferredLayer::render_indirect_lighting() const {
        indirect_pass_fbo->bind_as(GL_FRAMEBUFFER);
        OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        deferred_indirect->use();

        deferred_indirect->set_int(0, 0);
        deferred_indirect->set_int(1, 1);
        deferred_indirect->set_int(2, 2);
        deferred_indirect->set_int(3, 3);
        deferred_indirect->set_int(4, offsets_number);

        int next_texture_slot = 4;
        int location = 5;
        for(; next_texture_slot < 4 + 3 * number_of_lights; ++next_texture_slot, ++location){
            deferred_indirect->set_int(location, next_texture_slot);
        }
        for(int i = 0; i < number_of_lights; ++i, ++location){
            //TODO: right now, displacement radius is one value for both lights
            deferred_indirect->set_float(location, offset_displacement_radius);
        }

        gbuffer_positions_texture->bind_to_slot(0);
        gbuffer_normals_texture->bind_to_slot(1);
        gbuffer_diffuse_texture->bind_to_slot(2);
        offsets_texture->bind_to_slot(3);
        next_texture_slot = 4;
        for(auto&& position_rsm : rsm_position_vec){
            position_rsm->bind_to_slot(next_texture_slot);
            ++next_texture_slot;
        }
        for(auto&& normal_rsm : rsm_normal_vec){
            normal_rsm->bind_to_slot(next_texture_slot);
            ++next_texture_slot;
        }
        for(auto&& flux_rsm : rsm_flux_vec){
            flux_rsm->bind_to_slot(next_texture_slot);
            ++next_texture_slot;
        }
        OpenGL3_Renderer::draw(quad.vao);
        indirect_pass_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    void DeferredLayer::sum_lighting_components() const {
        OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        quad_render->use();
        quad_render->set_int(0, 0);
        direct_pass_output->bind_to_slot(0);
        OpenGL3_Renderer::draw(quad.vao);
        indirect_pass_output->bind_to_slot(0);
        OpenGL3_Renderer::draw(quad.vao);
        wireframe_overlay_output->bind_to_slot(0);
        OpenGL3_Renderer::draw(quad.vao);
    }

    void DeferredLayer::on_imgui_render() {
        ImGui::Begin("Light controls");

        for(int i = 0; i < number_of_lights; ++i){
            ImGui::Separator();
            ImGui::PushID(i);
            ImGui::Text("Light %d", i);
            glm::vec4 light_position = lights[i].get_position();
            glm::vec3 light_angles = lights[i].get_rotation_in_degrees();
            if (ImGui::DragFloat3("Light's Global Coordinates", glm::value_ptr(light_position), 0.01f, -5.0f, +5.0f,
                                  "%.3f")) {
                lights[i].translate_to(light_position);
                event_pump(std::make_unique<SceneChangedEvent>());
            }
            if (ImGui::DragFloat3("Light's Rotation Angles", glm::value_ptr(light_angles), 1.0f, 0.0f, 360.0f,
                                  "%.3f")) {
                lights[i].set_rotation(light_angles);
                event_pump(std::make_unique<SceneChangedEvent>());
            }
            if (ImGui::SliderFloat("Photometric Solid scaling", &scale_modifier[i], 0.00001f, 2.0f, "%.5f",
                                   ImGuiSliderFlags_Logarithmic)) {
                event_pump(std::make_unique<SceneChangedEvent>());
            }

            ImGui::Text("Photometric Solid size: %.5f", max_distance_to_ies_vertex[i] * scale_modifier[i]);
            ImGui::PopID();
        }
        ImGui::Separator();

        if(ImGui::SliderFloat("RSM Sampling displacement", &offset_displacement_radius, 0.0f, 2.0f)){
            event_pump(std::make_unique<SceneChangedEvent>());
        }
        if(ImGui::Checkbox("Show Photometric Solid", &draw_wireframe_in_scene)){
            event_pump(std::make_unique<SceneChangedEvent>());
        }
        if(draw_wireframe_in_scene){
            if(ImGui::ColorEdit4("Wireframe color", glm::value_ptr(wireframe_color))){
                event_pump(std::make_unique<SceneChangedEvent>());
            }
        }
        ImGui::End();
    }

    void DeferredLayer::indirect_pass_setup() {
        indirect_pass_fbo = std::make_unique<OpenGL3_FrameBuffer>();

        indirect_pass_output = OpenGL3_Texture2D_Builder()
                .with_size(target_resolution[0], target_resolution[1])
                .with_texture_format(GL_RGB16F)
                .with_data_format(GL_RGB)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .as_resource();

        const auto offsets = random_num::uniform_samples_on_unit_sphere(offsets_number);

        offsets_texture = OpenGL3_Texture1D_Builder()
                .with_size(offsets_number)
                .with_texture_format(GL_RGB32F)
                .with_data_format(GL_RGB)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .using_clamping_to_edge()
                .as_resource_with_data(offsets.data());

        indirect_pass_fbo->bind_as(GL_FRAMEBUFFER);
        indirect_pass_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *temp_depth_buffer);
        indirect_pass_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *indirect_pass_output);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        indirect_pass_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    void DeferredLayer::direct_pass_setup() {
        direct_pass_output = OpenGL3_Texture2D_Builder()
                .with_size(target_resolution[0], target_resolution[1])
                .with_texture_format(GL_RGB16F)
                .with_data_format(GL_RGB)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .as_resource();

        direct_pass_fbo = std::make_unique<OpenGL3_FrameBuffer>();
        direct_pass_fbo->bind_as(GL_FRAMEBUFFER);
        direct_pass_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                                     *temp_depth_buffer);
        direct_pass_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                     *direct_pass_output);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        direct_pass_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    void DeferredLayer::debug_setup() {
        constexpr std::array<float, 4> transparent_border{0.0f, 0.0f, 0.0f, 0.0f};
        wireframe_overlay_output = OpenGL3_Texture2D_Builder()
                .with_size(target_resolution[0], target_resolution[1])
                .with_texture_format(GL_RGBA)
                .with_data_format(GL_RGBA)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .using_clamping_to_borders()
                .using_border_color(transparent_border)
                .as_resource();

        debug_fbo = std::make_unique<OpenGL3_FrameBuffer>();
        debug_fbo->bind_as(GL_FRAMEBUFFER);
        debug_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                                          *gbuffer_depth_texture);
        debug_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                          *wireframe_overlay_output);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        debug_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    void DeferredLayer::gbuffer_creation_setup(const std::array<GLenum, 3>& color_attachments) {
        std::array<float, 4> white_border{1.0f, 1.0f, 1.0f, 1.0f};
        std::array<float, 4> black_border{0.0f, 0.0f, 0.0f, 1.0f};

        gbuffer_depth_texture = OpenGL3_Texture2D_Builder()
                .with_size(target_resolution[0], target_resolution[1])
                .with_texture_format(GL_DEPTH_COMPONENT)
                .with_data_format(GL_DEPTH_COMPONENT)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_minification()
                .using_linear_magnification()
                .using_clamping_to_borders()
                .using_border_color(white_border)
                .as_resource();


        gbuffer_positions_texture = OpenGL3_Texture2D_Builder()
                .with_size(target_resolution[0], target_resolution[1])
                .with_texture_format(GL_RGB16F)
                .with_data_format(GL_RGB)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .using_clamping_to_borders()
                .using_border_color(black_border)
                .as_resource();

        gbuffer_normals_texture = OpenGL3_Texture2D_Builder()
                .with_size(target_resolution[0], target_resolution[1])
                .with_texture_format(GL_RGB16F)
                .with_data_format(GL_RGB)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .using_clamping_to_borders()
                .using_border_color(black_border)
                .as_resource();

        gbuffer_diffuse_texture = OpenGL3_Texture2D_Builder()
                .with_size(target_resolution[0], target_resolution[1])
                .with_texture_format(GL_RGB)
                .with_data_format(GL_RGB)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .using_clamping_to_borders()
                .using_border_color(black_border)
                .as_resource();

        gbuffer_creation_fbo = std::make_unique<OpenGL3_FrameBuffer>();
        gbuffer_creation_fbo->bind_as(GL_FRAMEBUFFER);
        gbuffer_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                                          *gbuffer_depth_texture);
        gbuffer_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                          *gbuffer_positions_texture);
        gbuffer_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                                                          *gbuffer_normals_texture);
        gbuffer_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
                                                          *gbuffer_diffuse_texture);
        glDrawBuffers(3, color_attachments.data());
        gbuffer_creation_fbo->unbind_from(GL_FRAMEBUFFER);
    }


    void DeferredLayer::rsm_creation_setup(std::array<GLenum, 3>& color_attachments) {
        for(int i = 0; i < number_of_lights; ++i) {
            rsm_position_vec.push_back(OpenGL3_Cubemap_Builder().with_size(texture_resolution[0], texture_resolution[1])
                    .with_texture_format(GL_RGB16F)
                    .with_data_format(GL_RGB)
                    .using_underlying_data_type(GL_FLOAT)
                    .using_linear_magnification()
                    .using_linear_minification()
                    .as_resource());
            rsm_normal_vec.push_back(OpenGL3_Cubemap_Builder().with_size(texture_resolution[0], texture_resolution[1])
                    .with_texture_format(GL_RGB16F)
                    .with_data_format(GL_RGB)
                    .using_underlying_data_type(GL_FLOAT)
                    .using_linear_magnification()
                    .using_linear_minification()
                    .as_resource());
            rsm_flux_vec.push_back(OpenGL3_Cubemap_Builder().with_size(texture_resolution[0], texture_resolution[1])
                    .with_texture_format(GL_RGB)
                    .with_data_format(GL_RGB)
                    .using_underlying_data_type(GL_FLOAT)
                    .using_linear_magnification()
                    .using_linear_minification()
                    .as_resource());
        }
        rsm_creation_fbo = std::make_unique<OpenGL3_FrameBuffer>();
        rsm_creation_fbo->bind_as(GL_FRAMEBUFFER);
        rsm_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *rsm_depth_vec[0]);
        rsm_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *rsm_position_vec[0]);
        rsm_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, *rsm_normal_vec[0]);
        rsm_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, *rsm_flux_vec[0]);
        glDrawBuffers(3, color_attachments.data());
        rsm_creation_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    void DeferredLayer::ies_mask_creation_setup(std::array<GLenum, 3>& color_attachments) {
        for(int i = 0; i < number_of_lights; ++i){
            rsm_depth_vec.push_back(OpenGL3_Cubemap_Builder()
                                            .with_size(texture_resolution[0], texture_resolution[1])
                                            .with_texture_format(GL_DEPTH_COMPONENT)
                                            .with_data_format(GL_DEPTH_COMPONENT)
                                            .using_underlying_data_type(GL_FLOAT)
                                            .using_linear_magnification()
                                            .using_linear_minification()
                                            .as_resource());

            light_masks.push_back(OpenGL3_Cubemap_Builder()
                                          .with_size(texture_resolution[0], texture_resolution[1])
                                          .with_texture_format(GL_RGB16F)
                                          .with_data_format(GL_RGB)
                                          .using_underlying_data_type(GL_FLOAT)
                                          .using_linear_magnification()
                                          .using_linear_minification()
                                          .using_clamping_to_borders()
                                          .as_resource());
        }

        mask_creation_fbo = std::make_unique<OpenGL3_FrameBuffer>();
        mask_creation_fbo->bind_as(GL_FRAMEBUFFER);
        mask_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *rsm_depth_vec[0]);
        mask_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *light_masks[0]);
        glDrawBuffers(1, color_attachments.data());
        mask_creation_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    //TODO: fix sizes according to number_of_lights
    void DeferredLayer::uniform_buffers_setup() {
        gbuffer_transformation = std::make_shared<UniformBuffer>((4 * 4 * 4) * 3, GL_DYNAMIC_DRAW);
        gbuffer_transformation->bind_to_binding_point(0);
        gbuffer_transformation->unbind_from_uniform_buffer_target();

        material_buffer = std::make_shared<UniformBuffer>(16 + 4, GL_DYNAMIC_DRAW);
        material_buffer->bind_to_binding_point(1);
        material_buffer->unbind_from_uniform_buffer_target();

        //  sizeof(vec4) = 16, sizeof(float) = 4; (3 vec4s and 4 floats per light)
        light_buffer = std::make_shared<UniformBuffer>(((16 * 3) + (4 * 4)) * number_of_lights, GL_DYNAMIC_DRAW);
        light_buffer->bind_to_binding_point(2);
        light_buffer->unbind_from_uniform_buffer_target();

        //  sizeof(vec4) = 16, sizeof(float) = 4; 1 vec4,
        //  1 float (rounded up to 16) and (2 float per light) (rounded up to 16)
        //  see rule 4: https://community.khronos.org/t/std140-layout/71764/2
        common_buffer = std::make_shared<UniformBuffer>((16 * 1) + 16 + (16 * 2) * number_of_lights, GL_DYNAMIC_DRAW);
        common_buffer->bind_to_binding_point(3);
        common_buffer->bind_to_uniform_buffer_target();
        const auto view_camera = camera.lock();
        const auto camera_position_projective = glm::vec4(view_camera->position(), 1.0f);
        common_buffer->copy_to_buffer(0, 16, glm::value_ptr(camera_position_projective));
        common_buffer->copy_to_buffer(16, 4, &shadow_threshold);

        int start_offset = 32;
        //TODO: light_camera_far_plane could be different for each light
        //NOTE: because of the alignment rules for arrays, float[] have each element aligned to 16 bytes
        for(int i = 0; i < number_of_lights; ++i){
            common_buffer->copy_to_buffer(start_offset, 4, &light_camera_far_plane);
            start_offset += 16;
        }
        for(int i = 0; i < number_of_lights; ++i){
            const float ies_solid_scale = max_distance_to_ies_vertex[i] * scale_modifier[i];
            common_buffer->copy_to_buffer(start_offset, 4, &ies_solid_scale);
            start_offset += 16;
        }
        common_buffer->unbind_from_uniform_buffer_target();
    }


    std::vector<glm::mat4>
    DeferredLayer::compute_cubemap_view_projection_transforms(const glm::vec3& camera_position,
                                                              const glm::mat4& camera_projection_matrix) const {
        std::vector<glm::mat4> VP_transformation;
        VP_transformation.reserve(6);
        for (auto i = 0u; i < 6; ++i) {
            VP_transformation.push_back(camera_projection_matrix * glm::lookAt(
                    camera_position,
                    camera_position +
                    OpenGL3_Cubemap::directions[i],
                    OpenGL3_Cubemap::ups[i]));
        }
        return VP_transformation;
    }

    glm::mat4
    DeferredLayer::compute_light_model_matrix(const glm::vec3& light_position, const glm::mat4& light_orientation,
                                              float light_scale) const {
        auto ies_light_model_matrix = glm::mat4(1.0f);
        ies_light_model_matrix = glm::translate(ies_light_model_matrix, light_position);
        ies_light_model_matrix = ies_light_model_matrix * light_orientation;
        ies_light_model_matrix = glm::rotate(ies_light_model_matrix, glm::radians(90.0f),
                                             glm::vec3(1.0f, 0.0f, 0.0f));
        ies_light_model_matrix = glm::scale(ies_light_model_matrix, glm::vec3(light_scale));
        return ies_light_model_matrix;
    }

    void DeferredLayer::load_IES_light_as_VAO(const std::filesystem::path& path_to_IES_data, int light_index) {
        auto document = ies::IES_Default_Parser()
                .parse(path_to_IES_data.filename().string(), files::read_file(path_to_IES_data));
//        ies::adapter::IES_Mesh photometric_solid = ies::adapter::IES_Mesh::interpolate_from(document, 3);
        const auto photometric_solid = ies::adapter::IES_Mesh(document);

        const auto vertices = photometric_solid.get_vertices();

        max_distance_to_ies_vertex[light_index] = [](const std::vector<float>& vs) -> float {
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
        }(vertices);

        scale_modifier[light_index] = 1.5f / max_distance_to_ies_vertex[light_index];

        auto vbo = std::make_shared<VertexBuffer>(vertices.size() * sizeof(float),
                                                  vertices.data());
        vbo->set_buffer_layout(VertexBufferLayout({
                                                          VertexBufferElement(ShaderDataType::Float3,
                                                                              "position"),
                                                          VertexBufferElement(ShaderDataType::Float3,
                                                                              "normal")}));
        ies_light_vaos[light_index]->set_vbo(std::move(vbo));
        ies_light_vaos[light_index]->set_ebo(std::make_shared<ElementBuffer>(photometric_solid.get_indices()));
    }

    void DeferredLayer::update_camera_related_buffers() {
        common_buffer->bind_to_uniform_buffer_target();
        const auto view_camera = camera.lock();
        const auto camera_position_projective = glm::vec4(view_camera->position(), 1.0f);
        const auto projection_view_matrix = view_camera->projection_matrix() * view_camera->view_matrix();

        common_buffer->copy_to_buffer(0, 16, glm::value_ptr(camera_position_projective));
        common_buffer->unbind_from_uniform_buffer_target();
        gbuffer_transformation->bind_to_uniform_buffer_target();
        gbuffer_transformation->copy_to_buffer(0, 64, glm::value_ptr(projection_view_matrix));
        gbuffer_transformation->unbind_from_uniform_buffer_target();
    }

    void DeferredLayer::update_scene_buffers_and_representations() {
        constexpr auto light_intensity = 1.0f;
        constexpr auto light_color = glm::vec4(1.0f);

        //TODO: we can set a flag for updating only one light if only one is moving
        for(int i = 0; i < number_of_lights; ++i){
            const auto light_data = lights[i].get_representative_data();
            const auto light_position = glm::vec3(light_data.position);
            const auto light_orientation = glm::mat4_cast(lights[i].get_orientation());
            const auto light_camera = Camera(
                    CameraGeometricDefinition{light_data.position,
                                              light_data.position + light_data.direction,
                                              lights[i].get_up()},
                    90.0f, 1.0f,
                    CameraPlanes{0.001f, light_camera_far_plane},
                    CameraMode::Perspective);

            const auto light_projection_matrix = light_camera.get_projection_matrix();
            const auto light_transforms = compute_cubemap_view_projection_transforms(light_position,
                                                                                     light_projection_matrix);

            ies_model_matrices[i] = compute_light_model_matrix(light_position,
                                                            light_orientation, scale_modifier[i]);
            ies_inverse_transposed_matrices[i] = glm::transpose(glm::inverse(ies_model_matrices[i]));

            light_buffer->bind_to_uniform_buffer_target();
            light_buffer->copy_to_buffer(0 + 16 * i, 16, glm::value_ptr(light_data.position));
            light_buffer->copy_to_buffer(16 * number_of_lights + 16 * i, 16, glm::value_ptr(light_data.direction));
            light_buffer->copy_to_buffer(32 * number_of_lights + 16 * i + 0, 4, &lights[i].attenuation.constant);
            light_buffer->copy_to_buffer(32 * number_of_lights + 16 * i + 4, 4, &lights[i].attenuation.linear);
            light_buffer->copy_to_buffer(32 * number_of_lights + 16 * i + 8, 4, &lights[i].attenuation.quadratic);
            light_buffer->copy_to_buffer(32 * number_of_lights + 16 * i + 12, 4, &light_intensity);
            light_buffer->copy_to_buffer(48 * number_of_lights + 16 * i, 16, glm::value_ptr(light_color));
            light_buffer->unbind_from_uniform_buffer_target();

            update_light_mask(light_transforms, i);
            update_rsm(light_transforms, i);
        }
    }
}