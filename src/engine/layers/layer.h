#ifndef IES_PARSER_LAYER_H
#define IES_PARSER_LAYER_H

#include "../events/event.h"

namespace engine{
    class Layer {
    public:
        virtual ~Layer() = default;

        virtual void on_attach() = 0;
        virtual void on_detach() = 0;
        virtual void update([[maybe_unused]] float delta_time){};
        [[maybe_unused]] virtual void on_imgui_render(){};
        virtual void on_event(Event& event) = 0;
    };

}

#endif //IES_PARSER_LAYER_H
