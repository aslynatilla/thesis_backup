#ifndef SCENE_LAYER_H
#define SCENE_LAYER_H

#include "../layer.h"

#include "../../rendering/camera.h"
#include "../../rendering/renderer.h"
#include "../../rendering/shader_loading.h"
#include "../../rendering/opengl3_framebuffer.h"
#include "../../scene_management/scene_loading.h"
#include "../../scene_management/point_light.h"
#include "../../../utility/file_reader.h"
#include "../../../utility/random_numbers.h"

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
        PointLight point_light;
        std::shared_ptr<Shader> draw_shader;
        std::shared_ptr<Shader> rsm_generation_shader;
        std::vector<SceneObject> scene_objects;

        std::unique_ptr<OpenGL3_FrameBuffer> rsm_fbo;
        std::unique_ptr<OpenGL3_Cubemap> depth_cubemap;
        std::unique_ptr<OpenGL3_Cubemap> pos_cubemap;
        std::unique_ptr<OpenGL3_Cubemap> normal_cubemap;
        std::unique_ptr<OpenGL3_Cubemap> flux_cubemap;
        std::unique_ptr<OpenGL3_Cubemap> offset_mask_cubemap;

        std::unique_ptr<OpenGL3_Texture1D> sample_texture;
        std::vector<glm::vec3> polar_offsets;
        unsigned int sample_number;
    };
}

#endif //SCENE_LAYER_H
