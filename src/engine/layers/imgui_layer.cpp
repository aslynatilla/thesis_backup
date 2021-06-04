#include "imgui_layer.h"

namespace engine{
    ImGuiLayer::ImGuiLayer(GLFW_Window_Impl* main_window, [[maybe_unused]] LayerCreationKey key) : app_window{main_window}, is_blocking_events{false} {}

    void ImGuiLayer::on_attach() {

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplGlfw_InitForOpenGL(glfw_window_pointer(), true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void ImGuiLayer::on_detach() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::on_event(Event& event) {
        if (is_blocking_events)
        {
            ImGuiIO& io = ImGui::GetIO();
            event.handled |= /*event mouse related &*/ io.WantCaptureMouse;
            event.handled |= /*event keyboard related &*/ io.WantCaptureKeyboard;
        }
    }

    void ImGuiLayer::begin() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::end() {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(app_window->get_width()), static_cast<float>(app_window->get_height()));

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    GLFWwindow* ImGuiLayer::glfw_window_pointer() const {
        if(app_window != nullptr){
            return static_cast<GLFWwindow*>(app_window->get_native_window());
        }
        return nullptr;
    }

    std::unique_ptr<ImGuiLayer> ImGuiLayer::from(GLFW_Window_Impl* main_window) {
        return std::make_unique<ImGuiLayer>(main_window, LayerCreationKey{});
    }
}