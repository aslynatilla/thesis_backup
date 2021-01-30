#ifndef RENDERING_CONTEXT_H
#define RENDERING_CONTEXT_H

#include <memory>

namespace engine {
    class RenderingContext {
    public:
        virtual ~RenderingContext();

        virtual void initialize() = 0;

        virtual void swap_buffers() = 0;

        static std::unique_ptr<RenderingContext> create(void *window);
    };
}

#endif //RENDERING_CONTEXT_H
