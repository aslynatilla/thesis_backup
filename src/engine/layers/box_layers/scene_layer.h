#ifndef SCENE_LAYER_H
#define SCENE_LAYER_H

#include "../layer.h"

#include "../../rendering/camera.h"
#include "../../rendering/renderer.h"
#include "../../rendering/shader_loading.h"
#include "../../scene_management/scene_loading.h"
#include "../../scene_management/point_light.h"
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
        PointLight point_light;
        std::shared_ptr<Shader> shader;
        std::vector<SceneObject> scene_objects;
    };
}

#endif //SCENE_LAYER_H
