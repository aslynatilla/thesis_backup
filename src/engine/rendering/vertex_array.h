#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include "vertex_buffer.h"
#include "element_buffer.h"

#include <fmt/core.h>
#include <fmt/color.h>

#include <memory>

namespace engine{
    class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        VertexArray(const VertexArray& another_vao) = delete;
        VertexArray(VertexArray&& another_vao) = delete;
        VertexArray& operator=(const VertexArray& another_vao) = delete;
        VertexArray& operator=(VertexArray&& another_vao) = delete;

        void bind() const;
        void unbind() const;

        void set_vbo(std::shared_ptr<VertexBuffer>&& vertex_buffer_object);
        void set_ebo(std::shared_ptr<ElementBuffer>&& element_buffer_object);

        [[nodiscard]] const std::shared_ptr<VertexBuffer>& get_vbo() const;
        [[nodiscard]] const std::shared_ptr<ElementBuffer>& get_ebo() const;
    private:
        unsigned int id;
        unsigned int vbo_index;
        std::shared_ptr<VertexBuffer> vbo;
        std::shared_ptr<ElementBuffer> ebo;

        void load_layout_element(const VertexBufferElement& element,
                                 const unsigned int layout_stride, const unsigned int bonus_offset = 0u) const;
    };
}

#endif //VERTEX_ARRAY_H
