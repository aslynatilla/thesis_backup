#include "scene_events.h"

std::string engine::CameraMovedEvent::to_string() const {
    return std::string("Camera Moved Event");
}

std::string engine::SceneChangedEvent::to_string() const {
    return std::string("Scene Changed Event");
}
