#ifndef SCENE_LAYER_H
#define SCENE_LAYER_H

#include "../layer.h"

#include "../../events/window_events.h"
#include "../../events/keyboard_events.h"
#include "../../rendering/camera.h"
#include "../../rendering/renderer.h"
#include "../../rendering/shader_loading.h"
#include "../../rendering/opengl3_framebuffer.h"
#include "../../scene_management/scene_loading.h"
#include "../../scene_management/point_light.h"
#include "../../scene_management/spotlight.h"
#include "../../../utility/file_reader.h"
#include "../../../utility/random_numbers.h"

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

#include <glm/glm.hpp>

namespace engine{
    class SceneLayer : public Layer{
    public:
        void on_attach() final;
        void on_detach() final ;
        void on_event(Event& event) final;
        void update(float delta_time) final;
        void on_imgui_render() final;

    private:
        bool on_key_pressed(KeyPressedEvent event);
        bool set_light_in_shader(const SpotLight& light, std::shared_ptr<Shader>& shader);
        void draw_scene(std::shared_ptr<Shader>& shader,
                        const glm::mat4& light_view_matrix, const glm::mat4& light_projection_matrix);
        void bind_texture_in_slot(const unsigned int slot_number, OpenGL3_Texture1D* texture);
        void bind_texture_in_slot(const unsigned int slot_number, OpenGL3_Texture2D* texture);

        Camera view_camera;
        SpotLight scene_light;

        std::shared_ptr<Shader> draw_shader;
        std::shared_ptr<Shader> no_indirect_shader;
        std::shared_ptr<Shader> rsm_generation_shader;

        std::vector<SceneObject> scene_objects;

        std::unique_ptr<OpenGL3_FrameBuffer> rsm_fbo;
        std::unique_ptr<OpenGL3_Texture2D> depth_texture;
        std::unique_ptr<OpenGL3_Texture2D> position_texture;
        std::unique_ptr<OpenGL3_Texture2D> normal_texture;
        std::unique_ptr<OpenGL3_Texture2D> flux_texture;
        std::unique_ptr<OpenGL3_Texture1D> samples_texture;

        unsigned int samples_number;

        float light_intensity = 2.4f;
        float indirect_intensity = 25.0f;
        float max_radius = 0.24f;
        bool hide_direct_component = false;
        bool draw_indirect_light = true;

        std::array<unsigned int, 4> viewport_dimension;
        std::array<unsigned int, 2> texture_dimension;

        float timestep;
    };
}

#endif //SCENE_LAYER_H
