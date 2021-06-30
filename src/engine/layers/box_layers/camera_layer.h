#ifndef CAMERA_LAYER_H
#define CAMERA_LAYER_H

#include "../layer.h"

#include "../../events/window_events.h"
#include "../../events/keyboard_events.h"
#include "../../events/mouse_events.h"
#include "../../events/scene_events.h"
#include "../../rendering/fly_camera.h"

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

#include <glm/glm.hpp>

namespace engine{
    class CameraLayer : public Layer{
    public:
        template<typename FnCallback>
        static std::unique_ptr<CameraLayer> layer_for(std::shared_ptr<FlyCamera> camera, FnCallback event_pump_callback){
            auto layer = std::make_unique<CameraLayer>(std::move(camera), LayerCreationKey{});
            layer->event_pump = event_pump_callback;
            return layer;
        }
        CameraLayer(std::shared_ptr<FlyCamera> application_camera, LayerCreationKey key);

        CameraLayer() = delete;
        CameraLayer(const CameraLayer& other) = delete;
        CameraLayer(CameraLayer&& other) = delete;
        CameraLayer& operator=(const CameraLayer& other) = delete;
        CameraLayer& operator=(CameraLayer&& other) = delete;

        void on_attach() final;
        void on_detach() final;
        void on_event(Event& event) final;
        void update(float delta_time) final;
        void on_imgui_render() final;

    private:
        bool on_key_pressed(KeyPressedEvent event);
        bool on_mouse_moved(MouseMovedEvent event);
        bool on_mouse_button_pressed(MouseButtonPressedEvent event);
        bool on_mouse_button_released(MouseButtonReleasedEvent event);

        std::shared_ptr<FlyCamera> view_camera;
        std::function<void(std::unique_ptr<Event>)> event_pump;

        bool is_camera_rotating = false;

        float timestep;
        float translation_speed = 2.0f;
        float rotation_speed = 0.5f;
        glm::vec2 previous_mouse_position;
    };
}

#endif //CAMERA_LAYER_H
