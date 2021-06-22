#include "deferred_layer.h"

namespace engine {
    DeferredLayer::DeferredLayer(std::weak_ptr<FlyCamera> controlled_camera, [[maybe_unused]] LayerCreationKey key)
            : camera(std::move(controlled_camera)) {}

    void DeferredLayer::on_attach() {
        objects = default_load_scene("resources/cornell_box_multimaterial.obj");

        light = Point_Light(glm::vec4(1.5f, 2.6f, 1.5f, 1.0f),
                            LightAttenuationParameters{1.0f, 0.5f, 1.8f});
        light.set_rotation(glm::vec3(90.0f, 0.0f, 0.0f));

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
                                                      "resources/shaders/deferred/wireframe_to_gbuffer.frag");
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

        const auto path_to_IES_data = files::make_path_absolute("resources/ies/111621PN.IES");
        load_IES_light_as_VAO(path_to_IES_data);
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
        if (auto view_camera = camera.lock()) {

            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_DEPTH_TEST);

            if (scene_changed) {
                update_camera_related_buffers();
                update_scene_buffers_and_representations();
                create_gbuffer();
                //  conditionally, draw_wireframe()
                if (draw_wireframe_in_scene){
                    gbuffer_creation_fbo->bind_as(GL_FRAMEBUFFER);
                    glViewport(0, 0, target_resolution[0], target_resolution[1]);
                    wireframe_drawer->use();
                    gbuffer_diffuse_texture->bind_to_slot(2);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    gbuffer_transformation->bind_to_uniform_buffer_target();
                    gbuffer_transformation->copy_to_buffer(64, 4 * 4 * 4, glm::value_ptr(ies_model_matrix));
                    gbuffer_transformation->copy_to_buffer(128, 4 * 4 * 4, glm::value_ptr(ies_inverse_transposed_matrix));
                    gbuffer_transformation->unbind_from_uniform_buffer_target();
                    material_buffer->bind_to_uniform_buffer_target();
                    material_buffer->copy_to_buffer(0, 16, glm::value_ptr(wireframe_color));
                    material_buffer->unbind_from_uniform_buffer_target();
                    OpenGL3_Renderer::draw(ies_light_vao);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    gbuffer_creation_fbo->unbind_from(GL_FRAMEBUFFER);
                }
                scene_changed = false;
                camera_moved = false;
            } else if (camera_moved) {
                update_camera_related_buffers();
                create_gbuffer();
                if (draw_wireframe_in_scene){
                    gbuffer_creation_fbo->bind_as(GL_FRAMEBUFFER);
                    glViewport(0, 0, target_resolution[0], target_resolution[1]);
                    wireframe_drawer->use();
                    gbuffer_diffuse_texture->bind_to_slot(2);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    gbuffer_transformation->bind_to_uniform_buffer_target();
                    gbuffer_transformation->copy_to_buffer(64, 4 * 4 * 4, glm::value_ptr(ies_model_matrix));
                    gbuffer_transformation->copy_to_buffer(128, 4 * 4 * 4, glm::value_ptr(ies_inverse_transposed_matrix));
                    gbuffer_transformation->unbind_from_uniform_buffer_target();
                    material_buffer->bind_to_uniform_buffer_target();
                    material_buffer->copy_to_buffer(0, 16, glm::value_ptr(wireframe_color));
                    material_buffer->unbind_from_uniform_buffer_target();
                    OpenGL3_Renderer::draw(ies_light_vao);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    gbuffer_creation_fbo->unbind_from(GL_FRAMEBUFFER);
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

        for (const auto& o : objects) {
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
            OpenGL3_Renderer::draw(*(o.vao));
        }
        gbuffer_creation_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    void DeferredLayer::update_light_mask(const std::vector<glm::mat4>& light_transforms) {
        mask_creation_fbo->bind_as(GL_FRAMEBUFFER);
        glViewport(0, 0, texture_resolution[0], texture_resolution[1]);
        glCullFace(GL_FRONT);
        OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mask_creation->use();
        for (unsigned int i = 0u; i < 6; ++i) {
            mask_creation->set_mat4(0 + i,
                                    light_transforms[i]);
        }
        gbuffer_transformation->bind_to_uniform_buffer_target();
        gbuffer_transformation->copy_to_buffer(64, 64, glm::value_ptr(ies_model_matrix));
        gbuffer_transformation->unbind_from_uniform_buffer_target();
        OpenGL3_Renderer::draw(ies_light_vao);
        mask_creation_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    void
    DeferredLayer::update_rsm(const std::vector<glm::mat4>& light_transformations) {
        rsm_creation_fbo->bind_as(GL_FRAMEBUFFER);
        glViewport(0, 0, texture_resolution[0], texture_resolution[1]);
        glCullFace(GL_BACK);
        OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rsm_creation->use();
        rsm_positions->bind_to_slot(0);
        rsm_normals->bind_to_slot(1);
        rsm_fluxes->bind_to_slot(2);
        light_mask->bind_to_slot(3);
        rsm_creation->set_int(6, 3);

        for (int i = 0; i < 6; ++i) {
            rsm_creation->set_mat4(0 + i, light_transformations[i]);
        }

        for (const auto& o : objects) {
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
        deferred_direct->set_int(3, 3);
        deferred_direct->set_int(4, 4);
        gbuffer_positions_texture->bind_to_slot(0);
        gbuffer_normals_texture->bind_to_slot(1);
        gbuffer_diffuse_texture->bind_to_slot(2);
        shadow_map->bind_to_slot(3);
        light_mask->bind_to_slot(4);

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
        deferred_indirect->set_int(4, 4);
        deferred_indirect->set_int(5, 5);
        deferred_indirect->set_int(6, 6);
        deferred_indirect->set_int(10, offsets_number);
        deferred_indirect->set_float(11, offset_displacement_radius);
        gbuffer_positions_texture->bind_to_slot(0);
        gbuffer_normals_texture->bind_to_slot(1);
        gbuffer_diffuse_texture->bind_to_slot(2);
        rsm_positions->bind_to_slot(3);
        rsm_normals->bind_to_slot(4);
        rsm_fluxes->bind_to_slot(5);
        offsets_texture->bind_to_slot(6);
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
    }

    void DeferredLayer::on_imgui_render() {
        glm::vec4 light_position = light.get_position();
        glm::vec3 light_angles = light.get_rotation_in_degrees();

        ImGui::Begin("Light controls");
        if (ImGui::DragFloat3("Light's Global Coordinates", glm::value_ptr(light_position), 0.05f,  -0.5f, 3.5f, "%.3f")) {
            light.translate_to(light_position);
            event_pump(std::make_unique<SceneChangedEvent>());
        }
        if (ImGui::DragFloat3("Light's Rotation Angles", glm::value_ptr(light_angles), 1.0f, 0.0f, 360.0f, "%.3f")) {
            light.set_rotation(light_angles);
            event_pump(std::make_unique<SceneChangedEvent>());
        }
        if(ImGui::SliderFloat("Photometric Solid scaling", &scale_modifier, 0.00001f, 2.0f, "%.5f",
                           ImGuiSliderFlags_Logarithmic)){
            event_pump(std::make_unique<SceneChangedEvent>());
        }
        ImGui::Text("Photometric Solid size: %.5f", max_distance_to_ies_vertex * scale_modifier);
        ImGui::End();
    }

    std::vector<SceneObject> default_load_scene(const std::string& path_to_scene) {
        constexpr unsigned int postprocessing_flags = aiProcess_GenNormals |
                                                      aiProcess_Triangulate |
                                                      aiProcess_ValidateDataStructure;
        auto scene_objects = scenes::load_scene_objects_from(path_to_scene, postprocessing_flags);

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
        return scene_objects;
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
                .with_texture_format(GL_RGB32F)
                .with_data_format(GL_RGB)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .using_clamping_to_borders()
                .using_border_color(black_border)
                .as_resource();

        gbuffer_normals_texture = OpenGL3_Texture2D_Builder()
                .with_size(target_resolution[0], target_resolution[1])
                .with_texture_format(GL_RGB32F)
                .with_data_format(GL_RGB)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .using_clamping_to_borders()
                .using_border_color(black_border)
                .as_resource();

        gbuffer_diffuse_texture = OpenGL3_Texture2D_Builder()
                .with_size(target_resolution[0], target_resolution[1])
                .with_texture_format(GL_RGB32F)
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
        rsm_positions = OpenGL3_Cubemap_Builder().with_size(texture_resolution[0], texture_resolution[1])
                .with_texture_format(GL_RGB16F)
                .with_data_format(GL_RGB)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .as_resource();
        rsm_normals = OpenGL3_Cubemap_Builder().with_size(texture_resolution[0], texture_resolution[1])
                .with_texture_format(GL_RGB16F)
                .with_data_format(GL_RGB)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .as_resource();
        rsm_fluxes = OpenGL3_Cubemap_Builder().with_size(texture_resolution[0], texture_resolution[1])
                .with_texture_format(GL_RGB16F)
                .with_data_format(GL_RGB)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .as_resource();

        rsm_creation_fbo = std::make_unique<OpenGL3_FrameBuffer>();
        rsm_creation_fbo->bind_as(GL_FRAMEBUFFER);
        rsm_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *shadow_map);
        rsm_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *rsm_positions);
        rsm_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, *rsm_normals);
        rsm_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, *rsm_fluxes);
        glDrawBuffers(3, color_attachments.data());
        rsm_creation_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    void DeferredLayer::ies_mask_creation_setup(std::array<GLenum, 3>& color_attachments) {
        shadow_map = OpenGL3_Cubemap_Builder()
                .with_size(texture_resolution[0], texture_resolution[1])
                .with_texture_format(GL_DEPTH_COMPONENT)
                .with_data_format(GL_DEPTH_COMPONENT)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .as_resource();

        light_mask = OpenGL3_Cubemap_Builder()
                .with_size(texture_resolution[0], texture_resolution[1])
                .with_texture_format(GL_RGB16F)
                .with_data_format(GL_RGB)
                .using_underlying_data_type(GL_FLOAT)
                .using_linear_magnification()
                .using_linear_minification()
                .using_clamping_to_borders()
                .as_resource();

        mask_creation_fbo = std::make_unique<OpenGL3_FrameBuffer>();
        mask_creation_fbo->bind_as(GL_FRAMEBUFFER);
        mask_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *shadow_map);
        mask_creation_fbo->texture_to_attachment_point(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *light_mask);
        glDrawBuffers(1, color_attachments.data());
        mask_creation_fbo->unbind_from(GL_FRAMEBUFFER);
    }

    void DeferredLayer::uniform_buffers_setup() {
        gbuffer_transformation = std::make_shared<UniformBuffer>((4 * 4 * 4) * 3, GL_DYNAMIC_DRAW);
        gbuffer_transformation->bind_to_binding_point(0);
        gbuffer_transformation->unbind_from_uniform_buffer_target();

        material_buffer = std::make_shared<UniformBuffer>(16 + 4, GL_DYNAMIC_DRAW);
        material_buffer->bind_to_binding_point(1);
        material_buffer->unbind_from_uniform_buffer_target();

        light_buffer = std::make_shared<UniformBuffer>((16 * 3) + (4 * 4), GL_DYNAMIC_DRAW);
        light_buffer->bind_to_binding_point(2);
        light_buffer->unbind_from_uniform_buffer_target();

        common_buffer = std::make_shared<UniformBuffer>((16 * 1) + (4 * 3), GL_DYNAMIC_DRAW);
        common_buffer->bind_to_binding_point(3);
        common_buffer->bind_to_uniform_buffer_target();
        const auto view_camera = camera.lock();
        const auto camera_position_projective = glm::vec4(view_camera->position(), 1.0f);
        common_buffer->copy_to_buffer(0, 16, glm::value_ptr(camera_position_projective));
        common_buffer->copy_to_buffer(16, 4, &light_camera_far_plane);
        common_buffer->copy_to_buffer(20, 4, &shadow_threshold);
        const auto ies_solid_scale = max_distance_to_ies_vertex * scale_modifier;
        common_buffer->copy_to_buffer(24, 4, &ies_solid_scale);
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
    DeferredLayer::compute_light_model_matrix(const glm::vec3& light_position,
                                              const glm::mat4& light_orientation) const {
        auto ies_light_model_matrix = glm::mat4(1.0f);
        ies_light_model_matrix = glm::translate(ies_light_model_matrix, light_position);
        ies_light_model_matrix = ies_light_model_matrix * light_orientation;
        ies_light_model_matrix = glm::rotate(ies_light_model_matrix, glm::radians(90.0f),
                                             glm::vec3(1.0f, 0.0f, 0.0f));
        ies_light_model_matrix = glm::scale(ies_light_model_matrix, glm::vec3(scale_modifier));
        return ies_light_model_matrix;
    }

    void DeferredLayer::load_IES_light_as_VAO(const std::filesystem::path& path_to_IES_data) {
        auto document = ies::IES_Default_Parser()
                .parse(path_to_IES_data.filename().string(), files::read_file(path_to_IES_data));
        ies::adapter::IES_Mesh photometric_solid = ies::adapter::IES_Mesh::interpolate_from(document, 3);
//        const auto photometric_solid = ies::adapter::IES_Mesh(document);

        const auto vertices = photometric_solid.get_vertices();

        max_distance_to_ies_vertex = [](const std::vector<float>& vs) -> float {
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

        scale_modifier = 1.5f / max_distance_to_ies_vertex;

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

        const auto light_data = light.get_representative_data();
        const auto light_position = glm::vec3(light_data.position);
        const auto light_orientation = glm::mat4_cast(light.get_orientation());
        const auto light_camera = Camera(
                CameraGeometricDefinition{light_data.position,
                                          light_data.position + light_data.direction,
                                          light.get_up()},
                90.0f, 1.0f,
                CameraPlanes{0.001f, light_camera_far_plane},
                CameraMode::Perspective);

        const auto light_projection_matrix = light_camera.get_projection_matrix();
        const auto light_transforms = compute_cubemap_view_projection_transforms(light_position,
                                                                                 light_projection_matrix);

        ies_model_matrix = compute_light_model_matrix(light_position,
                                                      light_orientation);
        ies_inverse_transposed_matrix = glm::transpose(glm::inverse(ies_model_matrix));

        light_buffer->bind_to_uniform_buffer_target();
        light_buffer->copy_to_buffer(0, 16, glm::value_ptr(light_data.position));
        light_buffer->copy_to_buffer(16, 16, glm::value_ptr(light_data.direction));
        light_buffer->copy_to_buffer(32, 4, &light.attenuation.constant);
        light_buffer->copy_to_buffer(36, 4, &light.attenuation.linear);
        light_buffer->copy_to_buffer(40, 4, &light.attenuation.quadratic);
        light_buffer->copy_to_buffer(44, 4, &light_intensity);
        light_buffer->copy_to_buffer(48, 16, glm::value_ptr(light_color));
        light_buffer->unbind_from_uniform_buffer_target();

        update_light_mask(light_transforms);
        update_rsm(light_transforms);
    }
}