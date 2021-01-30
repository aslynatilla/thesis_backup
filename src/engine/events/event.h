#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <fmt/ostream.h>

namespace engine {

    enum class EventType {
        None = 0,
        WindowClosed, WindowResized,
        MouseMoved, MouseScrolled, MouseButtonPressed, MouseButtonReleased,
        KeyPressed, KeyReleased, InputKeyTyped,

        CameraReposition
    };

    class Event {
    public:
        virtual ~Event();

        virtual EventType get_event_type() const = 0;

        virtual std::string to_string() const;

        bool handled = false;
    };

    inline std::ostream& operator<<(std::ostream& os, const Event& printable_event);

    template<EventType type>
    class EventImpl : public Event {
    public:
        EventType get_event_type() const { return type; };

        static EventType get_static_type() { return type; };
    };

    class EventHandler {
    public:
        EventHandler(Event& to_be_handled);

        template<typename HandledType, typename HandlerFunction>
        bool handle(HandlerFunction f) {
            if (to_handle.get_event_type() == HandledType::get_static_type()) {
                to_handle.handled = f(static_cast<HandledType&>(to_handle));
                return true;
            }
            return false;
        }

    private:
        Event& to_handle;
    };

}
#endif //EVENT_H
