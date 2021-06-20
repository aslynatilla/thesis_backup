#ifndef SCENE_EVENTS_H
#define SCENE_EVENTS_H

#include "event.h"

namespace engine {
    class CameraMovedEvent : public EventImpl<EventType::CameraMoved> {
    public:
        std::string to_string() const override;
    };

    class SceneChangedEvent : public EventImpl<EventType::SceneChanged> {
    public:
        std::string to_string() const override;
    };
}
#endif //SCENE_EVENTS_H
