#include "camera_layer.h"

namespace engine{

    CameraLayer::CameraLayer(std::shared_ptr<FlyCamera> application_camera)
    : view_camera{application_camera}
    {}

    void CameraLayer::on_attach() {}

    void CameraLayer::on_detach() {}

    void CameraLayer::on_event(Event& event) {
        EventHandler handler(event);

        handler.handle<KeyPressedEvent>([this](auto&& ...args) -> decltype(auto) {
            return on_key_pressed(std::forward<decltype(args)>(args)...);
        });
        handler.handle<MouseMovedEvent>([this](auto&& ...args) -> decltype(auto) {
            return on_mouse_moved(std::forward<decltype(args)>(args)...);
        });
        handler.handle<MouseButtonPressedEvent>([this](auto&& ...args) -> decltype(auto) {
            return on_mouse_button_pressed(std::forward<decltype(args)>(args)...);
        });
        handler.handle<MouseButtonReleasedEvent>([this](auto&& ...args) -> decltype(auto) {
            return on_mouse_button_released(std::forward<decltype(args)>(args)...);
        });
        event.handled = false;
    }

    void CameraLayer::update(float delta_time) {
        timestep = delta_time;
        view_camera->update();
    }

    void CameraLayer::on_imgui_render() {
        Layer::on_imgui_render();
    }

    bool CameraLayer::on_key_pressed(KeyPressedEvent event) {
        const float translational_speed = 150.0f * timestep;

        glm::vec3 translation_vector(0.0f);
        if (event.get_keycode() == GLFW_KEY_D) {
            translation_vector += glm::vec3(-translational_speed, 0.0f, 0.0f);
        }
        if (event.get_keycode() == GLFW_KEY_A) {
            translation_vector += glm::vec3(translational_speed, 0.0f, 0.0f);
        }
        if (event.get_keycode() == GLFW_KEY_W) {
            translation_vector += glm::vec3(0.0f, 0.0f, translational_speed);
        }
        if (event.get_keycode() == GLFW_KEY_S) {
            translation_vector += glm::vec3(0.0f, 0.0f, -translational_speed);
        }
        if (event.get_keycode() == GLFW_KEY_Q) {
            translation_vector += glm::vec3(0.0f, translational_speed, 0.0f);
        }
        if (event.get_keycode() == GLFW_KEY_E) {
            translation_vector += glm::vec3(0.0f, -translational_speed, 0.0f);
        }

        view_camera->translate(translation_vector);
        return false;
    }

    bool CameraLayer::on_mouse_moved(MouseMovedEvent event) {
        const auto rotational_speed = 0.5f * timestep;
        const auto x = event.x();
        const auto y = event.y();
        if (is_camera_rotating) {
            view_camera->rotate_horizontally(-(x - previous_mouse_position.x) * rotational_speed);
            view_camera->rotate_vertically(-(y - previous_mouse_position.y) * rotational_speed);
        }
        previous_mouse_position.x = x;
        previous_mouse_position.y = y;
        return false;
    }

    bool CameraLayer::on_mouse_button_pressed(MouseButtonPressedEvent event) {
        if (event.get_button() == 1) {
            is_camera_rotating = true;
        }
        return false;
    }

    bool CameraLayer::on_mouse_button_released(MouseButtonReleasedEvent event) {
        if (event.get_button() == 1) {
            is_camera_rotating = false;
        }
        return false;
    }
}