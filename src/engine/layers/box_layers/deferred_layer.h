#ifndef DEFERRED_LAYER_H
#define DEFERRED_LAYER_H

#include "../layer.h"

#include "../../rendering/renderer.h"
#include "../../rendering/opengl3_framebuffer.h"
#include "../../rendering/opengl3_texture.h"
#include "../../rendering/fly_camera.h"
#include "../../rendering/shader_loading.h"
#include "../../scene_management/scene_loading.h"
#include "../../scene_management/point_light.h"

#include <glm/glm.hpp>

namespace engine{
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

        glm::vec<2, unsigned int> target_resolution;

        Point_Light light;
        std::vector<SceneObject> objects;

        std::unique_ptr<OpenGL3_FrameBuffer> gbuffer_creation_fbo;
        std::unique_ptr<OpenGL3_Texture2D> gbuffer_depth_texture;
        std::unique_ptr<OpenGL3_Texture2D> gbuffer_positions_texture;
        std::unique_ptr<OpenGL3_Texture2D> gbuffer_normals_texture;
        std::unique_ptr<OpenGL3_Texture2D> gbuffer_diffuse_texture;

        std::shared_ptr<Shader> gbuffer_creation;
        std::shared_ptr<Shader> deferred_pass;

        [[nodiscard]] std::vector<SceneObject> default_load_scene(const std::string& path_to_scene) const;
    };
}

#endif //DEFERRED_LAYER_H
