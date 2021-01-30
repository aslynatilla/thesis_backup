#include "event.h"

namespace engine {
    Event::~Event() {}

    std::string Event::to_string() const {
        return std::string("Base Event");
    }

    EventHandler::EventHandler(Event& to_be_handled) : to_handle{to_be_handled} {}

}