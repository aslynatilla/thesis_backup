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
        std::unique_ptr<OpenGL3_Texture> depth_texture;
        std::unique_ptr<OpenGL3_Texture> position_texture;
        std::unique_ptr<OpenGL3_Texture> normal_texture;
        std::unique_ptr<OpenGL3_Texture> flux_texture;
    };
}

#endif //SCENE_LAYER_H
