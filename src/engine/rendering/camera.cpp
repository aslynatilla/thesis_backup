#include "camera.h"

namespace engine {

    Camera::Camera() :
            definition({.position{0.0f, 0.0f, -10.0f},
                               .look_at_position{0.0f, 0.0f, 0.0f},
                               .up{0.0f, 1.0f, 0.0f}}),
            vertical_FOV_in_degrees(45.0f),
            aspect_ratio(1.0f),
            view_planes({.near_plane = 0.1f,
                                .far_plane = 100.0f}),
            projection_mode(CameraMode::Orthographic) {}


    Camera::Camera(const CameraGeometricDefinition& def, float vertical_fov, float aspect,
                   CameraPlanes planes, CameraMode mode) :
            definition(def),
            vertical_FOV_in_degrees(vertical_fov),
            aspect_ratio(aspect),
            view_planes(planes),
            projection_mode(mode) {}


    glm::vec3 Camera::get_position() const {
        return definition.position;
    }

    glm::vec3 Camera::get_direction() const {
        return glm::normalize(definition.look_at_position - definition.position);
    }

    glm::mat4 Camera::get_view_matrix() const {
        const auto new_look_at_position = definition.position + get_direction();
        return glm::lookAt(definition.position, new_look_at_position, definition.up);
    }

    glm::mat4 Camera::get_projection_matrix() const {
        if (projection_mode == CameraMode::Perspective) {
            return glm::perspective(glm::radians(vertical_FOV_in_degrees), aspect_ratio,
                                    view_planes.near_plane, view_planes.far_plane);
        } else { // projection_mode == CameraMode::Orthographic
            const float half_length = view_planes.far_plane / 2.0f;
            return glm::ortho(-aspect_ratio * half_length, aspect_ratio * half_length,
                              -half_length, half_length,
                              view_planes.near_plane, view_planes.far_plane);
        }
    }

    void Camera::reset_position(float height, float distance) {
        definition.position = glm::vec3(0.0f, height, distance);
        definition.look_at_position = glm::vec3(0.0f, height, 0.0f);
        definition.up = glm::vec3(0.0f, -1.0f, 0.0f);

        if (projection_mode == CameraMode::Orthographic) {
            view_planes.near_plane = 0.1f;
            view_planes.far_plane = distance * 2.0f;
        }
    }

    void Camera::rotate_around_vertical_axis(const float angle_in_degrees) {
        const float angle = glm::radians(angle_in_degrees);
        const auto displacement = definition.position - definition.look_at_position;
        const glm::quat rotation = glm::angleAxis(angle, definition.up);
        definition.position = definition.look_at_position + glm::rotate(rotation, displacement);
    }

    void Camera::rotate_around_horizontal_axis(const float angle_in_degrees) {
        const float angle = glm::radians(angle_in_degrees);
        const auto displacement = definition.position - definition.look_at_position;
        const auto axis = glm::cross(definition.up, glm::normalize(-displacement));
        const glm::quat rotation = glm::angleAxis(angle, axis);
        definition.position = definition.look_at_position + glm::rotate(rotation, displacement);
        definition.up = glm::rotate(rotation, definition.up);
    }

    void Camera::roll(const float angle_in_degrees) {
        const float angle = glm::radians(angle_in_degrees);
        const auto displacement = definition.position - definition.look_at_position;
        const auto axis = glm::normalize(-displacement);
        const glm::quat rotation = glm::angleAxis(angle, axis);
        definition.position = definition.look_at_position + glm::rotate(rotation, displacement);
        definition.up = glm::rotate(rotation, definition.up);
    }

    void Camera::on_event(Event& event) {
        EventHandler handler(event);
        handler.handle<WindowResizedEvent>([this](auto&& ... args) -> decltype(auto) {
            return on_window_resized(std::forward<decltype(args)>(args)...);
        });
    }

    bool Camera::on_window_resized(WindowResizedEvent& event) {
        aspect_ratio = static_cast<float>(event.get_target_width()) / static_cast<float>(event.get_target_height());
        return false;
    }

    void Camera::translate(const glm::vec3& translation) {
        if (translation.x != 0) {
            const auto forward = get_direction();
            const auto up = definition.up;
            const auto right = glm::cross(up, forward);
            definition.position += right * translation.x;
            definition.look_at_position += right * translation.x;
        }
        if (translation.y != 0) {
            definition.position += definition.up * translation.y;
            definition.look_at_position += definition.up * translation.y;
        }
        if (translation.z != 0) {
            const auto forward = get_direction();
            definition.position +=  forward * translation.z;
            definition.look_at_position += forward * translation.z;
        }
    }

    void Camera::local_rotate_x(const float rotation_speed) {
        const auto rotation = glm::angleAxis(rotation_speed, definition.up);
        const auto new_forward = glm::rotate(rotation, get_direction());
        definition.look_at_position = definition.position + new_forward;
    }

    void Camera::local_rotate_y(const float rotation_speed) {
        const auto rotation = glm::angleAxis(rotation_speed, glm::cross(definition.up, get_direction()));
        definition.up = glm::rotate(rotation, definition.up);
        const auto new_forward = glm::rotate(rotation, get_direction());
        definition.look_at_position = definition.position + new_forward;
    }
}