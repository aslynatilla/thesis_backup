#ifndef FLY_CAMERA_H
#define FLY_CAMERA_H

#include "camera.h"

namespace engine{
    struct FlyCameraState{
        glm::vec3 position;
        glm::vec3 up;
        glm::vec3 forward;
        glm::vec3 right;
        float angle_around_x;
        float angle_around_y;
    };

    struct CameraProjectionParameters{
        float aspect_ratio;
        float field_of_view;
        CameraPlanes planes;
    };

    class FlyCamera{
    public:
        FlyCamera() = default;
        FlyCamera(const glm::vec3 position, const float horizontal_rotation_in_radians, const float vertical_rotation_in_radians,
                  const CameraProjectionParameters& params,
                  const CameraMode mode = CameraMode::Perspective);

        void on_event(Event& e);
        void translate(const glm::vec3& translation);
        void rotate_vertically(const float angle_in_radians);
        void rotate_horizontally(const float angle_in_radians);
        void update();

        glm::vec3 position() const;
        glm::mat4 view_matrix() const;
        glm::mat4 projection_matrix() const;
        FlyCameraState state;
    private:
        CameraProjectionParameters projection_params;
        CameraMode projection_mode;

        bool on_window_resized(WindowResizedEvent& event);

        constexpr static float TWO_PI_VALUE = 2.0f * glm::pi<float>();
        float field_of_view_threshold;
    };
}

#endif //FLY_CAMERA_H
