#ifndef CAMERA_LAYER_H
#define CAMERA_LAYER_H

#include "../layer.h"

#include "../../events/window_events.h"
#include "../../events/keyboard_events.h"
#include "../../events/mouse_events.h"
#include "../../rendering/fly_camera.h"

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

#include <glm/glm.hpp>

namespace engine{
    class CameraLayer : public Layer{
    public:
        CameraLayer(std::shared_ptr<FlyCamera> application_camera);

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

        bool is_camera_rotating = false;

        float timestep;
        glm::vec2 previous_mouse_position;
    };
}

#endif //CAMERA_LAYER_H
