#include "deferred_layer.h"

namespace engine {
    DeferredLayer::DeferredLayer(std::weak_ptr<FlyCamera> controlled_camera)
            : camera(std::move(controlled_camera)) {}

    void DeferredLayer::on_attach() {
        objects = default_load_scene("resources/cornell_box_multimaterial.obj");

        light = Point_Light(glm::vec4(278.0f, 548.0f, 279.5f, 1.0f),
                            LightAttenuationParameters{1.0f, 0.004f, 0.00009f});
        light.set_rotation(glm::vec3(90.0f, 0.0f, 0.0f));

        auto viewport_size = std::make_unique<float[]>(4);
        glGetFloatv(GL_VIEWPORT, viewport_size.get());
        std::transform(&viewport_size[2], &viewport_size[4], glm::value_ptr(target_resolution),
                       [](const auto f) { return static_cast<unsigned int>(f); });

        auto create_empty_texture_handle = [](const GLenum texture_format,
                                              const GLenum data_format,
                                              const glm::vec<2, unsigned int> texture_size)
                -> std::unique_ptr<OpenGL3_Texture2D> {
            return std::make_unique<OpenGL3_Texture2D>(texture_format,
                                                       OpenGL3_TextureParameters(
                                                               {GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER,
                                                                GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T},
                                                               {GL_LINEAR, GL_LINEAR,
                                                                GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}),
                                                       texture_size[0], texture_size[1],
                                                       data_format, GL_FLOAT, nullptr);
        };

        gbuffer_depth_texture = create_empty_texture_handle(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, target_resolution);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(1.0f)));
        gbuffer_positions_texture = create_empty_texture_handle(GL_RGB32F, GL_RGB, target_resolution);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(0.0f)));
        gbuffer_normals_texture = create_empty_texture_handle(GL_RGB32F, GL_RGB, target_resolution);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(0.0f)));
        gbuffer_diffuse_texture = create_empty_texture_handle(GL_RGB32F, GL_RGB, target_resolution);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(0.0f)));
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
//        deferred_pass = shader::create_shader_from("resources/shaders/deferred/deferred.vert",
//                                                   "resources/shaders/deferred/deferred.frag");
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
            gbuffer_creation->use();
            gbuffer_creation->set_mat4("projection_view", projection_view_matrix);

            gbuffer_creation->set_int("gbuff_position", 0);
            gbuffer_positions_texture->make_active_in_slot(0);
            glBindTexture(gbuffer_positions_texture->bound_type, gbuffer_positions_texture->id);
            gbuffer_creation->set_int("gbuff_normal", 1);
            gbuffer_positions_texture->make_active_in_slot(1);
            glBindTexture(gbuffer_normals_texture->bound_type, gbuffer_normals_texture->id);
            gbuffer_creation->set_int("gbuff_diffuse", 2);
            gbuffer_positions_texture->make_active_in_slot(2);
            glBindTexture(gbuffer_diffuse_texture->bound_type, gbuffer_diffuse_texture->id);

            for (const auto& o : objects) {
                const auto& model_matrix = o.transform;
                const auto& transposed_inversed_model_matrix = o.transpose_inverse_transform;
                gbuffer_creation->set_mat4("model", model_matrix);
                gbuffer_creation->set_mat4("transposed_inversed_model", transposed_inversed_model_matrix);
                OpenGL3_Renderer::draw(*(o.vao));
            }
            gbuffer_creation_fbo->unbind_from(GL_FRAMEBUFFER);
        }

//      Notes about how to bind to a certain uniform block.
//        GLuint bindingPoint = 1, buffer, blockIndex;
//        float myFloats[8] = {1.0, 0.0, 0.0, 1.0,   0.4, 0.0, 0.0, 1.0};
//
//        blockIndex = glGetUniformBlockIndex(p, "ColorBlock");
//        glUniformBlockBinding(p, blockIndex, bindingPoint);
//
//        glGenBuffers(1, &buffer);
//        glBindBuffer(GL_UNIFORM_BUFFER, buffer);
//
//        glBufferData(GL_UNIFORM_BUFFER, sizeof(myFloats), myFloats, GL_DYNAMIC_DRAW);
//        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, buffer);
    }

    void DeferredLayer::on_imgui_render() {
        Layer::on_imgui_render();
    }

    std::vector<SceneObject> DeferredLayer::default_load_scene(const std::string& path_to_scene) const {
        constexpr unsigned int postprocessing_flags = aiProcess_GenNormals |
                                                      aiProcess_Triangulate |
                                                      aiProcess_ValidateDataStructure;
        return scenes::load_scene_objects_from(path_to_scene, postprocessing_flags);
    }
}