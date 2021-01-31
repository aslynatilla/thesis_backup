#include "box_app.h"

BoxApp::BoxApp() : Application("Cornell Box App"){
    using namespace engine;

    layers.push_layer(std::make_unique<SceneLayer>());

    const auto glfw_window_pointer = dynamic_cast<GLFW_Window_Impl*>(main_window.get());
    auto imgui_layer = std::make_unique<ImGuiLayer>(glfw_window_pointer);
    imgui = imgui_layer.get();
    layers.push_back_layer(std::move(imgui_layer));
}

BoxApp::~BoxApp() = default;

void BoxApp::on_event(engine::Event& event) {
    Application::on_event(event);
}

void BoxApp::run() {
    while (running) {
        auto time = static_cast<float>(glfwGetTime());
        float delta_time = time - last_frame_time;
        last_frame_time = time;

        for (auto& layer : layers) {
            layer->update(delta_time);
        }

        imgui->begin();
        for (auto& layer: layers) {
            layer->on_imgui_render();
        }
        imgui->end();

        main_window->on_update();
    }}

bool BoxApp::on_window_closed(engine::WindowClosedEvent& event) {
    return Application::on_window_closed(event);
}

bool BoxApp::on_window_resized(engine::WindowResizedEvent& event) {
    return Application::on_window_resized(event);
}
