#ifndef KEYBOARD_EVENTS_H
#define KEYBOARD_EVENTS_H

#include "event.h"

namespace engine {
    class KeyPressedEvent : public EventImpl<EventType::KeyPressed> {
    public:
        explicit KeyPressedEvent(const int pressed_key, const bool is_repeated);

        [[nodiscard]] int get_keycode() const;
        [[nodiscard]] bool is_pressed_repeatedly() const;

    private:
        int keycode;
        bool repeated;
    };

    class KeyReleasedEvent : public EventImpl<EventType::KeyReleased> {
    public:
        explicit KeyReleasedEvent(const int pressed_key);

        [[nodiscard]] int get_keycode() const;

    private:
        int keycode;
    };

    class KeyTypedEvent : public EventImpl<EventType::InputKeyTyped> {
    public:
        explicit KeyTypedEvent(const unsigned int typed_key);

        [[nodiscard]] unsigned int get_key_typed() const;
    private:
        unsigned int keycode;
    };
}

#endif //KEYBOARD_EVENTS_H
