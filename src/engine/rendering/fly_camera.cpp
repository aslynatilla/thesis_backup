#include "fly_camera.h"

namespace engine {
    FlyCamera::FlyCamera(const glm::vec3 position, const float horizontal_rotation_in_radians,
                         const float vertical_rotation_in_radians,
                         const engine::CameraProjectionParameters& params, const engine::CameraMode mode)
            : state(), projection_params(params), projection_mode(mode) {
        state.position = position;
        state.angle_around_y = horizontal_rotation_in_radians;
        state.angle_around_x = vertical_rotation_in_radians;
        update();
    }

    void FlyCamera::on_event(Event& e) {
        EventHandler handler(e);
        handler.handle<WindowResizedEvent>([this](auto&& ... args) -> decltype(auto) {
            return on_window_resized(std::forward<decltype(args)>(args)...);
        });
    }

    void FlyCamera::translate(const glm::vec3& translation) {
        state.position += translation.x * state.right +
                          translation.y * state.up +
                          translation.z * state.forward;
    }

    void FlyCamera::rotate_vertically(const float angle_in_radians) {
        state.angle_around_x = std::fmod(state.angle_around_x + angle_in_radians, TWO_PI_VALUE);
    }

    void FlyCamera::rotate_horizontally(const float angle_in_radians) {
        state.angle_around_y = std::fmod(state.angle_around_y + angle_in_radians, TWO_PI_VALUE);
    }

    bool FlyCamera::on_window_resized(engine::WindowResizedEvent& event) {
        projection_params.aspect_ratio = static_cast<float>(event.get_target_width())
                                         / static_cast<float>(event.get_target_height());
        return false;
    }

    void FlyCamera::update() {
        float cos_H = glm::cos(state.angle_around_y);
        float sin_H = glm::sin(state.angle_around_y);
        float cos_V = glm::cos(state.angle_around_x);
        float sin_V = glm::sin(state.angle_around_x);

//        state.forward = glm::vec3(cos_V * cos_H, sin_V, cos_V * sin_H);
//        state.up = glm::vec3(-sin_V * cos_H, cos_V, -sin_V * sin_H);
//        state.right = glm::cross(state.up, state.forward);

        state.forward = glm::vec3(cos_V * cos_H, sin_V, cos_V * sin_H);
        state.up = glm::vec3(-sin_V * cos_H, cos_V, -sin_V * sin_H);
        state.right = glm::cross(state.up, state.forward);
    }

    glm::vec3 FlyCamera::position() const {
        return state.position;
    }

    glm::mat4 FlyCamera::view_matrix() const {
        return glm::lookAt(state.position, state.position + state.forward, state.up);
    }

    glm::mat4 FlyCamera::projection_matrix() const {
        if (projection_mode == CameraMode::Perspective) {
            return glm::perspective(glm::radians(projection_params.field_of_view),
                                    projection_params.aspect_ratio,
                                    projection_params.planes.near_plane,
                                    projection_params.planes.far_plane);
        } else { // projection_mode == CameraMode::Orthographic
            const float half_length = projection_params.planes.far_plane / 2.0f;
            const float aspect = projection_params.aspect_ratio;
            return glm::ortho(-aspect * half_length, aspect * half_length,
                              -half_length, half_length,
                              projection_params.planes.near_plane,
                              projection_params.planes.far_plane);
        }
    }
}