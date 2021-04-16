#include "box_app.h"

BoxApp::BoxApp() : Application("Cornell Box App"){
    using namespace engine;

    std::shared_ptr first_person_camera = std::make_shared<FlyCamera>(
            glm::vec3{278.0f, 277.0f, -800.0f},
            glm::radians(0.0f),
            glm::radians(0.0f),
            CameraProjectionParameters{
                .aspect_ratio = 1.0f,
                .field_of_view = 45.0f,
                .planes = CameraPlanes{0.1f, 2000.0f}
            },
            CameraMode::Perspective);
    std::weak_ptr ptr_to_camera(first_person_camera);

    layers.push_layer(std::make_unique<CameraLayer>(std::move(first_person_camera)));
    layers.push_layer(std::make_unique<DeferredLayer>(std::move(ptr_to_camera)));

    const auto glfw_window_pointer = dynamic_cast<GLFW_Window_Impl*>(main_window.get());
    auto imgui_layer = std::make_unique<ImGuiLayer>(glfw_window_pointer);
    imgui = imgui_layer.get();
    layers.push_back_layer(std::move(imgui_layer));
}

BoxApp::~BoxApp(){
    fmt::print("[APPLICATION] {} terminating.", name);
}

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
