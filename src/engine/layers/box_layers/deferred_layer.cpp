#include "deferred_layer.h"

namespace engine {
    DeferredLayer::DeferredLayer(std::weak_ptr<FlyCamera> controlled_camera, [[maybe_unused]] LayerCreationKey key)
            : camera(std::move(controlled_camera)) {}

    void DeferredLayer::on_attach() {
        objects = default_load_scene("resources/cornell_box_multimaterial.obj");

        light = Point_Light(glm::vec4(278.0f, 548.0f, 279.5f, 1.0f),
                            LightAttenuationParameters{1.0f, 0.004f, 0.00009f});
        light.set_rotation(glm::vec3(90.0f, 0.0f, 0.0f));

        auto viewport_size = std::make_unique<float[]>(4);
        glGetFloatv(GL_VIEWPORT, viewport_size.get());
        std::transform(&viewport_size[2], &viewport_size[4], glm::value_ptr(target_resolution),
                       [](const auto f) { return static_cast<int>(f); });

        std::array<float, 4> white_border {1.0f, 1.0f, 1.0f, 1.0f};
        std::array<float, 4> black_border {0.0f, 0.0f, 0.0f, 1.0f};

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
        std::array<GLenum, 3> fbo_enums = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, &fbo_enums[0]);
        gbuffer_creation_fbo->unbind_from(GL_FRAMEBUFFER);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        gbuffer_creation = shader::create_shader_from("resources/shaders/deferred/gbuffer_creation.vert",
                                                      "resources/shaders/deferred/gbuffer_creation.frag");
        quad_render = shader::create_shader_from("resources/shaders/deferred/quad_rendering.vert",
                                                 "resources/shaders/deferred/quad_rendering.frag");
        deferred_direct = shader::create_shader_from("resources/shaders/deferred/quad_rendering.vert",
                                                     "resources/shaders/deferred/deferred_direct.frag");

        gbuffer_transformation = std::make_shared<UniformBuffer>((4 * 4 * 4) * 3, GL_DYNAMIC_DRAW);
        gbuffer_transformation->bind_to_binding_point(0);
        gbuffer_transformation->unbind_from_uniform_buffer_target();

        material_buffer = std::make_shared<UniformBuffer>(16 + 4, GL_DYNAMIC_DRAW);
        material_buffer->bind_to_binding_point(1);
        material_buffer->unbind_from_uniform_buffer_target();

        light_buffer = std::make_shared<UniformBuffer>((16*5)+ (4*4), GL_DYNAMIC_DRAW);
        light_buffer->bind_to_binding_point(2);
        light_buffer->unbind_from_uniform_buffer_target();

        common_buffer = std::make_shared<UniformBuffer>(3 * 4, GL_DYNAMIC_DRAW);
        common_buffer->bind_to_binding_point(3);
        common_buffer->unbind_from_uniform_buffer_target();
    }

    void DeferredLayer::on_detach() {

    }

    void DeferredLayer::on_event(Event& event) {
        event.handled = false;
    }

    void DeferredLayer::update(float delta_time) {
        [[maybe_unused]] float timestep = delta_time;
        if (auto view_camera = camera.lock()) {

            const auto projection_view_matrix = view_camera->projection_matrix() * view_camera->view_matrix();

            gbuffer_creation_fbo->bind_as(GL_FRAMEBUFFER);
            OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
            OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            create_gbuffer(projection_view_matrix);
            gbuffer_creation_fbo->unbind_from(GL_FRAMEBUFFER);

            OpenGL3_Renderer::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
            OpenGL3_Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            quad_render->use();
            quad_render->set_int(0, 0);
            gbuffer_positions_texture->bind_to_slot(0);
            OpenGL3_Renderer::draw(quad.vao);
        }
    }

    void DeferredLayer::create_gbuffer(glm::mat4 projection_view_matrix) {
        gbuffer_creation->use();

        gbuffer_creation->set_int("gbuff_position", 0);
        gbuffer_positions_texture->bind_to_slot(0);
        gbuffer_creation->set_int("gbuff_normal", 1);
        gbuffer_normals_texture->bind_to_slot(1);
        gbuffer_creation->set_int("gbuff_diffuse", 2);
        gbuffer_diffuse_texture->bind_to_slot(2);

        gbuffer_transformation->bind_to_uniform_buffer_target();
        gbuffer_transformation->copy_to_buffer(0, 64, glm::value_ptr(projection_view_matrix));
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
    }

    void DeferredLayer::on_imgui_render() {
        Layer::on_imgui_render();
    }

    std::vector<SceneObject> default_load_scene(const std::string& path_to_scene){
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

    std::unique_ptr<DeferredLayer> DeferredLayer::create_using(std::weak_ptr<FlyCamera> controlled_camera) {
        return std::make_unique<DeferredLayer>(std::move(controlled_camera), LayerCreationKey{});
    }
}