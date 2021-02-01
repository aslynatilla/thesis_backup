#include "camera.h"

namespace engine {

    Camera::Camera() :
            definition({.position{0.0f, 0.0f, -10.0f},
                        .look_at_position{0.0f, 0.0f, 0.0f},
                        .up{0.0f, 1.0f, 0.0f}}),
            FOV_in_degrees(45.0f),
            aspect_ratio(1.0f),
            view_planes({.near_plane = 0.1f,
                         .far_plane = 100.0f}),
            projection_mode(CameraMode::Orthographic) {}


    Camera::Camera(const CameraGeometricDefinition& def, float fov, float aspect,
                   CameraPlanes planes, CameraMode mode) :
                   definition(def),
                   FOV_in_degrees(fov),
                   aspect_ratio(aspect),
                   view_planes(planes),
                   projection_mode(mode) {}


    glm::vec3 Camera::get_position() const {
        return definition.position;
    }

    glm::vec3 Camera::get_direction() const {
        return definition.look_at_position - definition.position;
    }

    glm::mat4 Camera::get_view_matrix() const {
        return glm::lookAt(definition.position, definition.look_at_position, definition.up);
    }

    glm::mat4 Camera::get_projection_matrix() const {
        if (projection_mode == CameraMode::Perspective) {
            return glm::perspective(glm::radians(FOV_in_degrees), aspect_ratio,
                                    view_planes.near_plane, view_planes.far_plane);
        } else { // projection_mode == CameraMode::Orthographic
            const float half_length = view_planes.far_plane / 2.0f;
            return glm::ortho(-aspect_ratio * half_length, aspect_ratio * half_length,
                              - half_length,  half_length,
                              view_planes.near_plane, view_planes.far_plane);
        }
    }

    void Camera::reset_position(float height, float distance) {
        definition.position = glm::vec3(0.0f, height, distance);
        definition.look_at_position = glm::vec3(0.0f, height, 0.0f);
        definition.up = glm::vec3(0.0f, -1.0f, 0.0f);

        if(projection_mode == CameraMode::Orthographic){
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
}