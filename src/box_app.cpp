#include "box_app.h"

BoxApp::BoxApp() : Application("Cornell Box App")
{
    using namespace engine;

    std::shared_ptr first_person_camera = std::make_shared<FlyCamera>(
        glm::vec3{0.5f, 0.2f, 0.0f},
        glm::radians(0.0f),
        glm::radians(-90.0f),
        CameraProjectionParameters{
            .aspect_ratio = 1.0f,
            .field_of_view = 45.0f,
            .planes = CameraPlanes{0.1f, 100.0f}},
        CameraMode::Perspective);
    std::weak_ptr ptr_to_camera(first_person_camera);

    layers.push_layer(CameraLayer::layer_for(first_person_camera, [this](auto ptr_to_queue) {
        push_event(std::move(ptr_to_queue));
    }));

    layers.push_layer(DeferredLayer::create_using(ptr_to_camera, [this](auto ptr_to_queue) {
        push_event(std::move(ptr_to_queue));
    }));

//    layers.push_layer(SceneLayer::create_using(ptr_to_camera, [this](auto ptr_to_queue) {
//        push_event(std::move(ptr_to_queue));
//    }));

    const auto glfw_window_pointer = GLFW_Window_Impl::convert_from(main_window.get());
    imgui = ImGuiLayer::from(glfw_window_pointer);
    imgui->on_attach();
}

BoxApp::~BoxApp()
{
    imgui->on_detach();
    fmt::print("[APPLICATION] {} terminating.", name);
}

void BoxApp::on_event(engine::Event &event)
{
    Application::on_event(event);
    if (!event.handled){
        imgui->on_event(event);
    }
}

void BoxApp::run()
{
    while (running)
    {
        auto time = static_cast<float>(glfwGetTime());
        float delta_time = time - last_frame_time;
        last_frame_time = time;
        while(!received_events.empty()){
            on_event(*received_events.front());
            received_events.pop();
        }

        for (auto &layer : layers){
            layer->update(delta_time);
        }
        imgui->update(delta_time);

        imgui->begin();
        for (auto &layer : layers){
            layer->on_imgui_render();
        }
        imgui->end();

        main_window->on_update();
    }
}

bool BoxApp::on_window_closed(engine::WindowClosedEvent &event)
{
    return Application::on_window_closed(event);
}

bool BoxApp::on_window_resized(engine::WindowResizedEvent &event)
{
    return Application::on_window_resized(event);
}