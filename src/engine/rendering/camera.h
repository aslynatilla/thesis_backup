#ifndef CAMERA_H
#define CAMERA_H

#include "../events/window_events.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace engine{

    struct CameraGeometricDefinition{
        glm::vec3 position;
        glm::vec3 look_at_position;
        glm::vec3 up;
    };

    struct CameraPlanes{
        float near_plane;
        float far_plane;
    };

    enum class CameraMode : bool {
        Perspective = false, Orthographic = true
    };

    class Camera{
    public:
        Camera();
        Camera(const CameraGeometricDefinition& def, float vertical_fov = 45.0f, float aspect = 1.0f,
               CameraPlanes planes = {0.1f, 100.0f},
               CameraMode mode = CameraMode::Orthographic);

        void rotate_around_vertical_axis(const float angle_in_degrees);
        void rotate_around_horizontal_axis(const float angle_in_degrees);
        void roll(const float angle_in_degrees);

        void translate(const glm::vec3& translation);
        void local_rotate_x(const float rotation_speed);
        void local_rotate_y(const float rotation_speed);

        glm::vec3 get_position() const;
        glm::vec3 get_direction() const;
        glm::mat4 get_view_matrix() const;
        glm::mat4 get_projection_matrix() const;

        void reset_position(float height, float distance);

        void on_event(Event& event);
    private:
        CameraGeometricDefinition definition;
        float       vertical_FOV_in_degrees;
        float       aspect_ratio;
        CameraPlanes view_planes;
        CameraMode projection_mode;

        bool on_window_resized(WindowResizedEvent& event);
    };
}


#endif //CAMERA_H
