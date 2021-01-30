#include "keyboard_events.h"

namespace engine{

    KeyPressedEvent::KeyPressedEvent(const int pressed_key, const bool is_repeated) : keycode{pressed_key}, repeated{is_repeated} {}

    int KeyPressedEvent::get_keycode() const {
        return this->keycode;
    }

    bool KeyPressedEvent::is_pressed_repeatedly() const {
        return this->repeated;
    }

    KeyReleasedEvent::KeyReleasedEvent(const int pressed_key) : keycode{pressed_key} {}

    int KeyReleasedEvent::get_keycode() const {
        return this->keycode;
    }

    KeyTypedEvent::KeyTypedEvent(const unsigned int typed_key) : keycode{typed_key} {}

    unsigned int KeyTypedEvent::get_key_typed() const {
        return this->keycode;
    }
}