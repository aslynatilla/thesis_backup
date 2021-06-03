#ifndef IMGUI_LAYER_H
#define IMGUI_LAYER_H

#include "layer.h"
#include "../events/window_events.h"
#include "../rendering/glfw_window_impl.h"

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

namespace engine {
    class ImGuiLayer : public Layer {
    public:
        static std::unique_ptr<ImGuiLayer> from(GLFW_Window_Impl* main_window);
        ImGuiLayer(GLFW_Window_Impl* main_window, LayerCreationKey key);

        void on_attach() override;
        void on_detach() override;
        void on_event(Event& event) override;

        void begin();
        void end();
    private:
        GLFWwindow* glfw_window_pointer() const;

        GLFW_Window_Impl* app_window;
        bool is_blocking_events;
    };
}

#endif //IMGUI_LAYER_H
