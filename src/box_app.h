#ifndef BOX_APP_H
#define BOX_APP_H

#include "application.h"
#include "engine/layers/imgui_layer.h"
#include "engine/layers/box_layers/scene_layer.h"
#include "engine/layers/box_layers/camera_layer.h"

class BoxApp : public Application {
public:
    BoxApp();
    virtual ~BoxApp();
    virtual void on_event(engine::Event& event) final override;
    virtual void run() final override;
    virtual bool on_window_closed([[maybe_unused]] engine::WindowClosedEvent& event) final override;
    virtual bool on_window_resized(engine::WindowResizedEvent& event) final override;

private:
    std::unique_ptr<engine::ImGuiLayer> imgui;
};


#endif //BOX_APP_H
