#ifndef DEFERRED_LAYER_H
#define DEFERRED_LAYER_H

#include "../layer.h"

#include "../../rendering/renderer.h"
#include "../../rendering/opengl3_framebuffer.h"
#include "../../rendering/opengl3_texture.h"
#include "../../rendering/fly_camera.h"
#include "../../rendering/shader_loading.h"
#include "../../rendering/uniform_buffer.h"
#include "../../scene_management/scene_loading.h"
#include "../../scene_management/point_light.h"

#include "../../../ies/ies_default_parser.h"
#include "../../../ies/adapter/ies_mesh.h"

#include <glm/glm.hpp>

namespace engine{
    struct RenderingQuad {
        std::array<glm::vec2, 8> vertex_data;

        VertexArray vao;

        RenderingQuad() :
                vertex_data({glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f),
                             glm::vec2(1.0f, -1.0f), glm::vec2(1.0f, 0.0f),
                             glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 1.0f),
                             glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 1.0f)}) {

            auto vbo = std::make_shared<VertexBuffer>(vertex_data.size() * sizeof(float) * 2,
                                                      vertex_data.data());
            vbo->set_buffer_layout(VertexBufferLayout({
                                                              VertexBufferElement(ShaderDataType::Float2, "vertex_position"),
                                                              VertexBufferElement(ShaderDataType::Float2, "vertex_uv")
                                                      }));
            vao.set_vbo(std::move(vbo));
            vao.set_ebo(std::make_shared<ElementBuffer>(std::vector<unsigned int>{0, 1, 2, 2, 3, 0}));
        }
    };

    class DeferredLayer : public Layer{
    public:
        DeferredLayer(std::weak_ptr<FlyCamera> controlled_camera, LayerCreationKey key);
        static std::unique_ptr<DeferredLayer> create_using(std::weak_ptr<FlyCamera> controlled_camera);
        DeferredLayer() = delete;
        DeferredLayer(const DeferredLayer& other) = delete;
        DeferredLayer(DeferredLayer&& other) = delete;
        DeferredLayer& operator=(const DeferredLayer& other) = delete;
        DeferredLayer& operator=(DeferredLayer&& other) = delete;

        void on_attach() override;
        void on_detach() override;
        void on_event(Event& event) override;
        void update(float delta_time) override;
        void on_imgui_render() override;

    private:
        std::weak_ptr<FlyCamera> camera;

        bool moving_camera = false;

        glm::vec<2, int> target_resolution {0, 0};
        float shadow_threshold = 0.15f;
        float max_distance_to_ies_vertex = 1.0f;
        VertexArray ies_light_vao;

        Point_Light light;
        std::vector<SceneObject> objects;
        RenderingQuad quad;

        std::unique_ptr<OpenGL3_FrameBuffer> gbuffer_creation_fbo;
        std::unique_ptr<OpenGL3_Texture2D> gbuffer_depth_texture;
        std::unique_ptr<OpenGL3_Texture2D> gbuffer_positions_texture;
        std::unique_ptr<OpenGL3_Texture2D> gbuffer_normals_texture;
        std::unique_ptr<OpenGL3_Texture2D> gbuffer_diffuse_texture;

        std::unique_ptr<OpenGL3_FrameBuffer> mask_creation_fbo;
        std::unique_ptr<OpenGL3_Cubemap> light_mask;

        std::unique_ptr<OpenGL3_FrameBuffer> rsm_creation_fbo;
        std::unique_ptr<OpenGL3_Cubemap> rsm_positions;
        std::unique_ptr<OpenGL3_Cubemap> rsm_normals;
        std::unique_ptr<OpenGL3_Cubemap> rsm_fluxes;
        std::unique_ptr<OpenGL3_Cubemap> shadow_map;

        std::unique_ptr<OpenGL3_FrameBuffer> direct_pass_fbo;
        std::unique_ptr<OpenGL3_Texture2D> direct_pass_output;

        std::shared_ptr<Shader> gbuffer_creation;   //  Should update when camera moves or scene changes
        std::shared_ptr<Shader> mask_creation;      //  Should update when light moves or scene changes
        std::shared_ptr<Shader> rsm_creation;       //  Should update when light moves or scene changes
        std::shared_ptr<Shader> deferred_direct;
        std::shared_ptr<Shader> quad_render;

        std::shared_ptr<UniformBuffer> gbuffer_transformation;
        std::shared_ptr<UniformBuffer> material_buffer;
        std::shared_ptr<UniformBuffer> light_buffer;
        std::shared_ptr<UniformBuffer> common_buffer;

        float light_camera_far_plane = 10.0f;

        void create_gbuffer(glm::mat4 projection_view_matrix);
        void render_direct_lighting();
        void update_rsm(const std::vector<glm::mat4>& light_transformations);

        void gbuffer_creation_setup(const std::array<GLenum, 3>& color_attachments);
        void ies_mask_creation_setup(std::array<GLenum, 3>& color_attachments);
        void rsm_creation_setup(std::array<GLenum, 3>& color_attachments);
        void direct_pass_setup();
        void uniform_buffers_setup();


        [[nodiscard]] std::vector<glm::mat4> compute_cubemap_view_projection_transforms(const glm::vec3& camera_position,
                                                                          const glm::mat4& camera_projection_matrix) const;

        [[nodiscard]] glm::mat4 compute_light_model_matrix(const glm::vec3& light_position,
                                                           const glm::mat4& light_orientation) const;

        //TODO: refactor this better, so that it can be a free function
        void load_IES_light_as_VAO(const std::filesystem::path& path_to_IES_data);
        bool on_camera_moved();

        void update_light_mask(const std::vector<glm::mat4>& light_transforms, const glm::mat4& ies_light_model_matrix);
    };


    [[nodiscard]] std::vector<SceneObject> default_load_scene(const std::string& path_to_scene);
}

#endif //DEFERRED_LAYER_H
