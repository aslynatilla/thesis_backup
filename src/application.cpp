#include "application.h"

Application* Application::instance = nullptr;

Application::Application(const std::string& app_name) {
    fmt::print("[APPLICATION] {} started.\n", app_name);
    if(instance != nullptr){
        fmt::print("[APPLICATION - ERROR] Trying to instantiate more than one application.\n");
    }
    instance = this;

    using namespace engine;
    main_window = Window::create(WindowProperties(app_name, 800, 800));
    main_window->set_event_callback([this](auto&& event) { return on_event(event); });
    OpenGL3_Renderer::initialize();
    running = true;
}

Application::~Application() {}

void Application::on_event(engine::Event& event) {
    using namespace engine;
    EventHandler handler(event);

    handler.handle<WindowClosedEvent>([this](auto&& ... args) -> decltype(auto) {
        return on_window_closed(std::forward<decltype(args)>(args)...);
    });

    handler.handle<WindowResizedEvent>([this](auto&& ... args) -> decltype(auto) {
        return on_window_resized(std::forward<decltype(args)>(args)...);
    });

    for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
        if (event.handled) {
            break;
        } else {
            (*it)->on_event(event);
        }
    }
}

void Application::run() {
    while (running) {
        auto time = static_cast<float>(glfwGetTime());
        float delta_time = time - last_frame_time;
        last_frame_time = time;

        for (auto& layer : layers) {
            layer->update(delta_time);
        }

        main_window->on_update();
    }
}

Application& Application::reference() {
    return *instance;
}

bool Application::on_window_closed([[maybe_unused]] engine::WindowClosedEvent& event) {
    running = false;
    return true;
}

bool Application::on_window_resized(engine::WindowResizedEvent& event) {
    engine::OpenGL3_Renderer::set_viewport(0, 0, event.get_target_width(), event.get_target_height());
    return false;
}