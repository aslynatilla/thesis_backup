#ifndef SCENE_LAYER_H
#define SCENE_LAYER_H

#include "../layer.h"

#include "../../events/window_events.h"
#include "../../events/keyboard_events.h"
#include "../../events/mouse_events.h"
#include "../../rendering/fly_camera.h"
#include "../../rendering/renderer.h"
#include "../../rendering/shader_loading.h"
#include "../../rendering/opengl3_framebuffer.h"
#include "../../rendering/uniform_buffer.h"
#include "../../scene_management/scene_loading.h"
#include "../../scene_management/point_light.h"
#include "../../scene_management/spotlight.h"
#include "../../../utility/file_reader.h"
#include "../../../utility/random_numbers.h"

#include "../../../ies/ies_default_parser.h"
#include "../../../ies/adapter/ies_mesh.h"

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

#include <glm/glm.hpp>

namespace engine {
    class SceneLayer : public Layer {
    public:
        SceneLayer(std::weak_ptr<FlyCamera> application_camera);

        SceneLayer() = delete;

        SceneLayer(const SceneLayer& other) = delete;

        SceneLayer(SceneLayer&& other) = delete;

        SceneLayer& operator=(const SceneLayer& other) = delete;

        SceneLayer& operator=(SceneLayer&& other) = delete;

        void on_attach() final;

        void on_detach() final;

        void on_event(Event& event) final;

        void update(float delta_time) final;

        void on_imgui_render() final;

    private:
        bool on_key_pressed(KeyPressedEvent event);

        void draw_scene(const glm::vec3 camera_position,
                        const glm::mat4 view_matrix,
                        const glm::mat4 projection_matrix,
                        std::shared_ptr<Shader>& shader);

        [[nodiscard]] glm::mat4
        compute_light_model_matrix(const glm::vec3 position,
                                   const glm::mat4 orientation_matrix,
                                   const float scale) const;

        [[nodiscard]] std::vector<glm::mat4>
        compute_VP_transform_for_cubemap(const glm::vec3 view_position,
                                         const glm::mat4 projection_matrix) const;

        std::weak_ptr<FlyCamera> view_camera;

        Point_Light scene_light;

        std::shared_ptr<Shader> draw_shader;
        std::shared_ptr<Shader> no_indirect_shader;
        std::shared_ptr<Shader> rsm_generation_shader;
        std::shared_ptr<Shader> wireframe_shader;
        std::shared_ptr<Shader> depthmask_shader;

        std::vector<SceneObject> scene_objects;
        std::vector<SceneObject> angel;

        std::unique_ptr<OpenGL3_FrameBuffer> rsm_fbo;
        std::unique_ptr<OpenGL3_FrameBuffer> mask_fbo;
        std::unique_ptr<OpenGL3_FrameBuffer> fake_default_fbo;
        std::unique_ptr<OpenGL3_Texture2D> fake_texture;
        std::unique_ptr<OpenGL3_Texture2D> fake_depth;
        std::unique_ptr<OpenGL3_Cubemap> depth_texture;
        std::unique_ptr<OpenGL3_Cubemap> position_texture;
        std::unique_ptr<OpenGL3_Cubemap> normal_texture;
        std::unique_ptr<OpenGL3_Cubemap> flux_texture;
        std::unique_ptr<OpenGL3_Texture1D> samples_texture;
        std::unique_ptr<OpenGL3_Cubemap> ies_light_mask;

        int VPL_samples_per_fragment;

        float light_intensity = 1.00f;
        glm::vec4 light_color = glm::vec4(1.0f);
        float indirect_intensity = 1.00f;
        float displacement_sphere_radius = 1.00f;
        float shadow_threshold = 0.15f;
        float light_far_plane = 200.00f;
        bool hide_direct_component = false;
        bool draw_indirect_light = true;
        bool ies_light_wireframe = true;
        glm::vec4 wireframe_color = glm::vec4(0.20f, 1.00f, 1.00f, 0.10f);
        bool is_using_ies_masking = true;

        std::array<int, 4> viewport_dimension;
        std::array<int, 2> texture_dimension;
        std::array<std::string, 6> light_transforms_strings;

        ies::IES_Default_Parser parser;
        ies::IES_Document document;
        VertexArray ies_light_vao;

        float largest_position_component;
        float scale_modifier = 0.00193f;

        std::shared_ptr<UniformBuffer> light_data_buffer;
        std::shared_ptr<UniformBuffer> matrices_buffer;
        std::shared_ptr<UniformBuffer> material_buffer;
        std::shared_ptr<UniformBuffer> common_data_buffer;
    };
}

#endif //SCENE_LAYER_H
