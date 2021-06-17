#include "camera_layer.h"

namespace engine{

    CameraLayer::CameraLayer(std::shared_ptr<FlyCamera> application_camera, [[maybe_unused]] LayerCreationKey key)
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
        const float scaled_speed = translation_speed * timestep;

        glm::vec3 translation_vector(0.0f);
        if (event.get_keycode() == GLFW_KEY_D) {
            translation_vector += glm::vec3(-scaled_speed, 0.0f, 0.0f);
        }
        if (event.get_keycode() == GLFW_KEY_A) {
            translation_vector += glm::vec3(scaled_speed, 0.0f, 0.0f);
        }
        if (event.get_keycode() == GLFW_KEY_W) {
            translation_vector += glm::vec3(0.0f, 0.0f, scaled_speed);
        }
        if (event.get_keycode() == GLFW_KEY_S) {
            translation_vector += glm::vec3(0.0f, 0.0f, -scaled_speed);
        }
        if (event.get_keycode() == GLFW_KEY_Q) {
            translation_vector += glm::vec3(0.0f, scaled_speed, 0.0f);
        }
        if (event.get_keycode() == GLFW_KEY_E) {
            translation_vector += glm::vec3(0.0f, -scaled_speed, 0.0f);
        }

        view_camera->translate(translation_vector);

        return false;
    }

    bool CameraLayer::on_mouse_moved(MouseMovedEvent event) {
        const auto scaled_speed = rotation_speed * timestep;
        const auto x = event.x();
        const auto y = event.y();
        if (is_camera_rotating) {
            view_camera->rotate_horizontally(-(x - previous_mouse_position.x) * scaled_speed);
            view_camera->rotate_vertically(-(y - previous_mouse_position.y) * scaled_speed);
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

    std::unique_ptr<CameraLayer> CameraLayer::layer_for(std::shared_ptr<FlyCamera> camera) {
        return std::make_unique<CameraLayer>(std::move(camera), LayerCreationKey{});
    }
}