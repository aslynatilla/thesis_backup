#ifndef SCENE_LAYER_H
#define SCENE_LAYER_H

#include "../layer.h"

#include "../../rendering/camera.h"
#include "../../rendering/renderer.h"
#include "../../rendering/shader_loading.h"
#include "../../rendering/opengl3_framebuffer.h"
#include "../../scene_management/scene_loading.h"
#include "../../scene_management/point_light.h"
#include "../../scene_management/spotlight.h"
#include "../../../utility/file_reader.h"
#include "../../../utility/random_numbers.h"

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
        Camera view_camera;
        SpotLight scene_light;

        std::shared_ptr<Shader> draw_shader;
        std::shared_ptr<Shader> rsm_generation_shader;
        std::vector<SceneObject> scene_objects;

        std::unique_ptr<OpenGL3_FrameBuffer> rsm_fbo;
        std::unique_ptr<OpenGL3_Texture2D> depth_texture;
        std::unique_ptr<OpenGL3_Texture2D> position_texture;
        std::unique_ptr<OpenGL3_Texture2D> normal_texture;
        std::unique_ptr<OpenGL3_Texture2D> flux_texture;
        std::unique_ptr<OpenGL3_Texture1D> samples_texture;

        unsigned int samples_number;

        float light_intensity = 1.0;
        float indirect_intensity = 1.0f;
        float max_radius = 40.0f;

        std::array<unsigned int, 4> viewport_dimension;
        std::array<unsigned int, 2> texture_dimension;
    };
}

#endif //SCENE_LAYER_H
