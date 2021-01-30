#ifndef APPLICATION_H
#define APPLICATION_H

#include "engine/rendering/renderer.h"              //  contains glad
#include "engine/rendering/glfw_window_impl.h"      //  contains glfw
#include "engine/events/window_events.h"
#include "engine/events/keyboard_events.h"
#include "engine/layers/layer_container.h"

#include <fmt/core.h>
#include <fmt/color.h>
#include <utility>

class Application {
public:
    Application(const std::string& app_name = "Cornell Box");

    virtual ~Application();
    virtual void on_event(engine::Event& event);
    virtual void run();

    static Application& reference();

protected:
    virtual bool on_window_closed([[maybe_unused]] engine::WindowClosedEvent& event);
    virtual bool on_window_resized(engine::WindowResizedEvent& event);

    std::string name;

    bool running;
    float last_frame_time;

    std::unique_ptr<engine::Window> main_window;
    engine::LayerContainer layers;

    static Application* instance;
};

#endif //APPLICATION_H
