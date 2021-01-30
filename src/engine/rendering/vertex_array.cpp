#include "vertex_array.h"

namespace engine{

    VertexArray::VertexArray() : id{0}, vbo_index{0} {
        glGenVertexArrays(1, &id);
    }

    VertexArray::~VertexArray() {
        glDeleteVertexArrays(1, &id);
    }

    void VertexArray::bind() const {
        glBindVertexArray(id);
    }

    void VertexArray::unbind() const {
        glBindVertexArray(0);
    }

    void VertexArray::set_vbo(std::shared_ptr<VertexBuffer>&& vertex_buffer_object) {
        glBindVertexArray(id);
        vbo_index = 0;
        vbo = std::move(vertex_buffer_object);
        vbo->bind();

        const auto& layout = vbo->get_buffer_layout();
        for(const VertexBufferElement& element : layout){
            switch (element.type) {
                case ShaderDataType::Bool:
                case ShaderDataType::Float:
                case ShaderDataType::Float2:
                case ShaderDataType::Float3:
                case ShaderDataType::Float4:
                case ShaderDataType::Int:
                case ShaderDataType::Int2:
                case ShaderDataType::Int3:
                case ShaderDataType::Int4: {
                    load_layout_element(element, layout.get_stride());
                    break;
                }
                case ShaderDataType::Mat3:
                case ShaderDataType::Mat4: {
                    const auto count = element.component_count();
                    for (auto i = 0u; i < count; ++i) {
                        load_layout_element(element, layout.get_stride(), sizeof(float) * count * i);
                    }
                    break;
                }
                case ShaderDataType::None:
                    fmt::print(fg(fmt::color::red), "[VERTEX ARRAY] Unknown ShaderDataType!");
            }
            ++vbo_index;
        }
    }

    void VertexArray::set_ebo(std::shared_ptr<ElementBuffer>&& element_buffer_object) {
        glBindVertexArray(id);
        element_buffer_object->bind();
        ebo = std::move(element_buffer_object);
    }

    const std::shared_ptr<VertexBuffer>& VertexArray::get_vbo() const {
        return vbo;
    }

    const std::shared_ptr<ElementBuffer>& VertexArray::get_ebo() const {
        return ebo;
    }

    void VertexArray::load_layout_element(const VertexBufferElement& element, const unsigned int layout_stride,
                                          const unsigned int bonus_offset) const {
        glEnableVertexAttribArray(vbo_index);
        glVertexAttribPointer(vbo_index,
                              element.component_count(),
                              datatype_to_GLenum_type(element.type),
                              element.normalized ? GL_TRUE : GL_FALSE,
                              layout_stride,
                              reinterpret_cast<const void*>(element.offset + bonus_offset));

    }
}